import io
import json
import unittest
from unittest.mock import patch
import urllib.error

import diagnose


CONFIG = {"provider": "gemini", "model": "gemini-3.8-flash", "key": "fake-secret"}
METADATA = {"name": "models/gemini-3.8-flash", "supportedGenerationMethods": ["generateContent"]}
OK = {"status": "completed", "steps": [{"type": "model_output", "status": "done",
       "content": [{"type": "text", "text": "OK"}]}]}


def http(code):
    body = json.dumps({"error": {"status": "UNAVAILABLE", "message": "high demand"}}).encode()
    return urllib.error.HTTPError("https://generativelanguage.googleapis.com/", code, "error", {}, io.BytesIO(body))


class DiagnosticTests(unittest.TestCase):
    def test_metadata_failure_prevents_generation(self):
        with patch.object(diagnose.review, "post_once", side_effect=http(403)) as call:
            result = diagnose.diagnose(CONFIG, [CONFIG["key"]])
        self.assertEqual(call.call_count, 1)
        self.assertEqual(result["status"], "error")
        self.assertEqual(call.call_args.args[0].get_method(), "GET")

    def test_success_selects_api_without_requesting_other_endpoint(self):
        with patch.object(diagnose.review, "post_once", side_effect=[METADATA, OK]) as call:
            result = diagnose.diagnose(CONFIG, [CONFIG["key"]])
        self.assertEqual(result["selected_api"], "interactions")
        self.assertEqual(call.call_count, 2)
        body = json.loads(call.call_args.args[0].data)
        self.assertFalse(body["store"])
        self.assertEqual(body["generation_config"]["max_output_tokens"], 1024)
        self.assertEqual(body["generation_config"]["thinking_level"], "low")

    def test_two_unavailable_endpoints_are_not_retried(self):
        with patch.object(diagnose.review, "post_once", side_effect=[METADATA, http(503), http(503)]) as call:
            result = diagnose.diagnose(CONFIG, [CONFIG["key"]])
        self.assertEqual(call.call_count, 3)
        self.assertEqual(result["status"], "error")
        self.assertEqual([c["api"] for c in result["checks"][1:]], ["interactions", "generate-content"])
        self.assertEqual(result["checks"][1]["provider_message"], "high demand")

    def test_quota_or_permission_stops_without_fallback(self):
        for code in (400, 401, 403, 429):
            with self.subTest(code=code), patch.object(diagnose.review, "post_once", side_effect=[METADATA, http(code)]) as call:
                result = diagnose.diagnose(CONFIG, [CONFIG["key"]])
            self.assertEqual(call.call_count, 2)
            self.assertEqual(result["status"], "error")

    def test_explicit_hydrogen_fallback_only_after_two_capacity_errors(self):
        with patch.object(diagnose.review, "post_once", side_effect=[METADATA, http(503), http(503), METADATA, OK]) as call:
            result = diagnose.with_fallback(CONFIG, [CONFIG["key"]], "gemini-4-hydrogen")
        self.assertEqual(call.call_count, 5)
        self.assertEqual(result["selected_model"], "gemini-4-hydrogen")
        self.assertEqual(result["selected_api"], "interactions")
        self.assertEqual(json.loads(call.call_args.args[0].data)["model"], "gemini-4-hydrogen")

    def test_no_implicit_model_switch_or_quota_bypass(self):
        for fallback, code in ((None, 503), ("gemini-4-hydrogen", 429), ("gemini-4-hydrogen", 403)):
            with self.subTest(fallback=fallback, code=code), patch.object(diagnose.review, "post_once", side_effect=[METADATA, http(code), http(code)]) as call:
                result = diagnose.with_fallback(CONFIG, [CONFIG["key"]], fallback)
            self.assertNotIn("alternative", result)
            self.assertEqual(result["status"], "error")
            self.assertLessEqual(call.call_count, 3)
