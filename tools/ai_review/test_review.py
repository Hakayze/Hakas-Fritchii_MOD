import argparse
from contextlib import redirect_stdout, redirect_stderr
import io
import json
from pathlib import Path
import tempfile
import threading
import unittest
from unittest.mock import patch
import urllib.error

import review


ENV = {"GEMINI_API_KEY": "fake-gemini-secret", "GEMINI_MODEL": "gemini-test",
       "DEEPSEEK_API_KEY": "fake-deepseek-secret", "DEEPSEEK_MODEL": "deepseek-test"}


def answer(provider, text="No demonstrated fault", finish=None):
    if provider == "gemini":
        return {"candidates": [{"content": {"parts": [{"text": "private thought", "thought": True}, {"text": text}]},
                                "finishReason": finish or "STOP"}]}
    return {"choices": [{"message": {"content": text, "reasoning_content": "private thought"},
                          "finish_reason": finish or "stop"}]}


class ReviewTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.input = self.root / "review.diff"
        self.input.write_text("diff --git a/a.cpp b/a.cpp\n+selected code\n")
        self.stdout, self.stderr = io.StringIO(), io.StringIO()

    def run_cli(self, extra=None, env=None):
        args = ["--input", str(self.input), "--output-dir", str(self.root / "reports"), "--provider", "both"]
        with redirect_stdout(self.stdout), redirect_stderr(self.stderr):
            return review.main(args + (extra or []), ENV if env is None else env)

    def test_payloads_and_authorization(self):
        a, b = review.configs(ENV, review.PROVIDERS)
        gem = review.make_request(a, "only selected text", 2048)
        deep = review.make_request(b, "only selected text", 2048)
        self.assertEqual(gem.method, "POST")
        self.assertEqual(gem.get_header("X-goog-api-key"), ENV["GEMINI_API_KEY"])
        self.assertNotIn(ENV["GEMINI_API_KEY"], gem.full_url)
        self.assertEqual(json.loads(gem.data)["generationConfig"]["maxOutputTokens"], 2048)
        self.assertEqual(deep.get_header("Authorization"), "Bearer " + ENV["DEEPSEEK_API_KEY"])
        self.assertFalse(json.loads(deep.data)["stream"])

    def test_two_providers_run_concurrently_and_save_final_only(self):
        barrier = threading.Barrier(2, timeout=5)
        def fake(request, timeout):
            barrier.wait()
            return answer("gemini" if "googleapis" in request.full_url else "deepseek")
        with patch.object(review, "post_once", side_effect=fake) as post:
            self.assertEqual(self.run_cli(), 0)
        self.assertEqual(post.call_count, 2)
        for provider in review.PROVIDERS:
            self.assertNotIn("private thought", (self.root / "reports" / (provider + ".md")).read_text())
        summary = json.loads((self.root / "reports/summary.json").read_text())
        self.assertEqual([x["status"] for x in summary["results"]], ["ok", "ok"])

    def test_missing_configuration_prevents_every_post(self):
        env = dict(ENV)
        del env["DEEPSEEK_MODEL"]
        with patch.object(review, "post_once") as post:
            self.assertEqual(self.run_cli(env=env), 1)
            post.assert_not_called()
        self.assertFalse((self.root / "reports").exists())

    def test_check_presence_only(self):
        with patch.object(review, "read_input") as source, patch.object(review, "post_once") as post:
            self.assertEqual(self.run_cli(["--check"]), 0)
            source.assert_not_called()
            post.assert_not_called()
        for value in ENV.values():
            self.assertNotIn(value, self.stdout.getvalue())

    def test_dry_run_without_keys_or_network_or_files(self):
        with patch.object(review, "post_once") as post:
            self.assertEqual(self.run_cli(["--dry-run"], env={}), 0)
            post.assert_not_called()
        self.assertFalse((self.root / "reports").exists())
        self.assertNotIn("selected code", self.stdout.getvalue())

    def test_input_cap_and_active_secret_rejected(self):
        with patch.object(review, "post_once") as post:
            self.assertEqual(self.run_cli(["--max-input-bytes", "1"]), 1)
            self.input.write_text("+" + ENV["GEMINI_API_KEY"])
            self.assertEqual(self.run_cli(), 1)
            post.assert_not_called()
        self.assertNotIn(ENV["GEMINI_API_KEY"], self.stderr.getvalue())

    def test_http_failure_single_attempt_and_other_result_preserved(self):
        def fake(request, timeout):
            if "googleapis" in request.full_url:
                raise urllib.error.HTTPError(request.full_url, 429, "quota", {}, None)
            return answer("deepseek")
        with patch.object(review, "post_once", side_effect=fake) as post:
            self.assertEqual(self.run_cli(), 1)
        self.assertEqual(post.call_count, 2)
        self.assertIn("No demonstrated fault", (self.root / "reports/deepseek.md").read_text())
        self.assertIn("HTTP 429", (self.root / "reports/gemini.md").read_text())

    def test_token_truncation_not_success(self):
        def fake(request, timeout):
            return answer("gemini", finish="MAX_TOKENS") if "googleapis" in request.full_url else answer("deepseek", finish="length")
        with patch.object(review, "post_once", side_effect=fake):
            self.assertEqual(self.run_cli(), 1)
        summary = json.loads((self.root / "reports/summary.json").read_text())
        self.assertEqual([x["status"] for x in summary["results"]], ["incomplete", "incomplete"])

    def test_output_redaction_and_local_truncation(self):
        def fake(request, timeout):
            return answer("gemini" if "googleapis" in request.full_url else "deepseek", ENV["GEMINI_API_KEY"] + "x" * 100)
        with patch.object(review, "post_once", side_effect=fake):
            self.assertEqual(self.run_cli(["--max-output-chars", "30"]), 1)
        for file in (self.root / "reports").iterdir():
            self.assertNotIn(ENV["GEMINI_API_KEY"], file.read_text())
        self.assertIn("incomplete", self.stdout.getvalue())

    def test_existing_directory_prevents_post(self):
        (self.root / "reports").mkdir()
        with patch.object(review, "post_once") as post:
            self.assertEqual(self.run_cli(), 1)
            post.assert_not_called()

    def test_no_redirect_and_http_size_json_checks(self):
        self.assertIsNone(review.NoRedirect().redirect_request(None, None, 302, "", {}, "https://elsewhere.invalid"))
        class Response(io.BytesIO):
            pass
        request = review.make_request(review.configs(ENV, ("gemini",))[0], "diff", 10)
        for data in (b"invalid-json", b"x" * (review.MAX_RESPONSE_BYTES + 1)):
            with patch.object(review.urllib.request, "build_opener") as factory:
                factory.return_value.open.return_value = Response(data)
                with self.assertRaises(review.ReviewError):
                    review.post_once(request, 1)
                self.assertIsInstance(factory.call_args.args[0], review.NoRedirect)
                self.assertEqual(factory.return_value.open.call_count, 1)

    def test_timeout_and_argument_errors_redact_keys(self):
        options = argparse.Namespace(max_output_tokens=10, timeout=1, max_output_chars=100)
        c = review.configs(ENV, ("gemini",))[0]
        with patch.object(review, "post_once", side_effect=TimeoutError(ENV["GEMINI_API_KEY"])) as post:
            result = review.review_one(c, "diff", options, [ENV["GEMINI_API_KEY"]])
            self.assertEqual(result["status"], "error")
            self.assertNotIn(ENV["GEMINI_API_KEY"], result["error"])
            self.assertEqual(post.call_count, 1)
        self.assertEqual(self.run_cli(["--invalid", ENV["GEMINI_API_KEY"]]), 1)
        self.assertNotIn(ENV["GEMINI_API_KEY"], self.stderr.getvalue())

    def test_models_have_no_default_and_paths_rejected(self):
        env = dict(ENV, GEMINI_MODEL="../../unsafe")
        with self.assertRaises(review.ReviewError):
            review.configs(env, review.PROVIDERS)
        env = dict(ENV, GEMINI_MODEL="")
        with self.assertRaises(review.ReviewError):
            review.configs(env, review.PROVIDERS)

    def test_default_gemini_needs_no_deepseek_configuration(self):
        env = {k: v for k, v in ENV.items() if k.startswith("GEMINI_")}
        with patch.object(review, "post_once", return_value=answer("gemini")) as post:
            with redirect_stdout(self.stdout), redirect_stderr(self.stderr):
                code = review.main(["--input", str(self.input), "--output-dir", str(self.root / "reports")], env)
        self.assertEqual(code, 0)
        self.assertEqual(post.call_count, 1)
        self.assertIn("googleapis", post.call_args.args[0].full_url)
        self.assertFalse((self.root / "reports/deepseek.md").exists())

    def test_default_gemini_quota_does_not_retry_or_fall_back(self):
        def quota(request, timeout):
            self.assertIn("googleapis", request.full_url)
            raise urllib.error.HTTPError(request.full_url, 429, "quota", {}, None)
        with patch.object(review, "post_once", side_effect=quota) as post:
            with redirect_stdout(self.stdout), redirect_stderr(self.stderr):
                code = review.main(["--input", str(self.input), "--output-dir", str(self.root / "reports")], ENV)
        self.assertEqual(code, 1)
        self.assertEqual(post.call_count, 1)
        self.assertFalse((self.root / "reports/deepseek.md").exists())
        summary = json.loads((self.root / "reports/summary.json").read_text())
        self.assertEqual([r["provider"] for r in summary["results"]], ["gemini"])

    def test_unavailable_is_reported_without_retry_or_authentication_claim(self):
        error = urllib.error.HTTPError("https://generativelanguage.googleapis.com/", 503,
                                       "UNAVAILABLE", {}, None)
        with patch.object(review, "post_once", side_effect=error) as post:
            self.assertEqual(self.run_cli(["--provider", "gemini"]), 1)
        self.assertEqual(post.call_count, 1)
        summary = json.loads((self.root / "reports/summary.json").read_text())
        result = summary["results"][0]
        self.assertEqual(result["http_status"], 503)
        self.assertEqual(result["status"], "error")
        self.assertIn("temporariamente indisponível", result["error"])
        self.assertNotIn("credencial", result["error"])
        self.assertFalse((self.root / "reports/deepseek.md").exists())


if __name__ == "__main__":
    unittest.main()
