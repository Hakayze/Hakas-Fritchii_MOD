#!/usr/bin/env python3
"""Bounded Gemini connectivity diagnosis; no source code or alternate models."""

import argparse
import json
import os
from pathlib import Path
import sys
import time
import urllib.error
import urllib.request

import review


def diagnose(config, keys, timeout=45):
    result = {"model": config["model"], "status": "error", "checks": []}
    metadata = {"stage": "model_metadata"}
    result["checks"].append(metadata)
    request = urllib.request.Request(
        "https://generativelanguage.googleapis.com/v1beta/models/" + config["model"],
        headers={"x-goog-api-key": config["key"], "Accept": "application/json"})
    try:
        doc = review.post_once(request, timeout)
        methods = doc.get("supportedGenerationMethods") or []
        metadata.update(status="ok", http_status=200, name=doc.get("name"),
                        supported_methods=methods)
    except urllib.error.HTTPError as error:
        metadata.update(review.http_failure(error, keys))
        return result
    except Exception as error:
        metadata.update(status="error", error=review.redact(error, keys)[:1000])
        return result

    # One tiny request per API at most. The model and key never change.
    # Stop immediately on permission/quota errors; no paid fallback or retries.
    for api in ("interactions", "generate-content"):
        check = {"stage": "minimal_generation", "api": api}
        result["checks"].append(check)
        started = time.monotonic()
        try:
            request = review.make_request(config, "OK", 1024, api,
                                          "Teste de conexão. Responda apenas OK.")
            body = json.loads(request.data)
            if config["model"].startswith("gemini-3.8-"):
                if api == "interactions":
                    body["generation_config"]["thinking_level"] = "low"
                else:
                    body["generationConfig"]["thinkingConfig"] = {"thinkingLevel": "low"}
                request.data = json.dumps(body).encode()
            doc = review.post_once(request, timeout)
            answer, finish, usage, complete = review.parse_response("gemini", doc, api)
            check.update(status="ok" if complete else "incomplete", http_status=200,
                         finish_reason=finish, text=review.redact(answer, keys)[:100], usage=usage)
            if complete:
                result.update(status="ok", selected_api=api)
        except urllib.error.HTTPError as error:
            check.update(review.http_failure(error, keys))
        except Exception as error:
            check.update(status="error", error=review.redact(error, keys)[:1000])
        check["elapsed_seconds"] = round(time.monotonic() - started, 3)
        if result["status"] == "ok" or check.get("http_status") not in (404, 503):
            break
    return result


def main(argv=None, environ=None):
    env = os.environ if environ is None else environ
    keys = [env.get("GEMINI_API_KEY", "").strip()]
    cli = argparse.ArgumentParser(description=__doc__)
    cli.add_argument("--output-dir", type=Path, required=True)
    args = cli.parse_args(argv)
    try:
        config = review.configs(env, ("gemini",))[0]
        args.output_dir.mkdir(parents=True, mode=0o700, exist_ok=False)
        result = diagnose(config, keys)
        encoded = review.redact(json.dumps(result, ensure_ascii=False, indent=2), keys)
        (args.output_dir / "diagnostic.json").write_text(encoded + "\n", encoding="utf-8")
        print(encoded)
        if result["status"] == "ok" and env.get("GITHUB_OUTPUT"):
            with open(env["GITHUB_OUTPUT"], "a", encoding="utf-8") as output:
                output.write("api=" + result["selected_api"] + "\n")
        return 0 if result["status"] == "ok" else 1
    except (review.ReviewError, OSError) as error:
        print(review.redact(error, keys), file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
