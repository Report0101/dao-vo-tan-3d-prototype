"""Bounded fixture validator; no engine, simulation imports, or network access."""
from pathlib import Path
import copy
import json
import math

ROOT = Path(__file__).resolve().parents[1]


def validate(value, rule, at="root"):
    if "const" in rule and value != rule["const"]:
        raise ValueError(f"{at}: const mismatch")
    if "enum" in rule and value not in rule["enum"]:
        raise ValueError(f"{at}: enum mismatch")
    kind = rule.get("type")
    if isinstance(value, list) and (len(value) < rule.get("minItems", 0) or len(value) > rule.get("maxItems", math.inf)):
        raise ValueError(f"{at}: array bounds")
    if kind is None and "properties" in rule and isinstance(value, dict):
        for key, child in rule["properties"].items():
            if key in value:
                validate(value[key], child, f"{at}.{key}")
    if kind == "object":
        if not isinstance(value, dict):
            raise ValueError(f"{at}: expected object")
        if any(k not in value for k in rule.get("required", [])):
            raise ValueError(f"{at}: missing required field")
        props = rule.get("properties", {})
        if rule.get("additionalProperties") is False and set(value) - set(props):
            raise ValueError(f"{at}: unknown field")
        for key, item in value.items():
            if key in props:
                validate(item, props[key], f"{at}.{key}")
    elif kind == "array":
        if not isinstance(value, list):
            raise ValueError(f"{at}: expected array")
        if len(value) < rule.get("minItems", 0) or len(value) > rule.get("maxItems", math.inf):
            raise ValueError(f"{at}: array bounds")
        for i, item in enumerate(value):
            validate(item, rule.get("items", {}), f"{at}[{i}]")
    elif kind == "string":
        if not isinstance(value, str) or len(value) < rule.get("minLength", 0):
            raise ValueError(f"{at}: expected nonempty string")
    elif kind in ("integer", "number"):
        if isinstance(value, bool) or not isinstance(value, (int, float)) or not math.isfinite(value):
            raise ValueError(f"{at}: expected finite number")
        if kind == "integer" and not isinstance(value, int):
            raise ValueError(f"{at}: expected integer")
        if value < rule.get("minimum", -math.inf) or value > rule.get("maximum", math.inf):
            raise ValueError(f"{at}: numeric bounds")
    elif kind == "boolean" and not isinstance(value, bool):
        raise ValueError(f"{at}: expected boolean")
    for conditional in rule.get("allOf", []):
        # This local subset is deliberately limited to the authority discriminator.
        expected = conditional["if"]["properties"]["authority"]["properties"]["source"]["const"]
        if value.get("authority", {}).get("source") == expected:
            validate(value, conditional["then"], at)
            authority_rule = conditional["then"]["properties"]["authority"]["properties"]
            for key, child in authority_rule.items():
                validate(value["authority"][key], child, f"{at}.authority.{key}")


def check_request(request, rule):
    validate(request, rule)
    for cue in request["cosmeticCues"]:
        if cue["offsetSeconds"] > request["visualDurationSeconds"]:
            raise ValueError("cue after visual duration")
    if request["authority"]["source"] == "simulation":
        if request["authority"]["nonAuthoritative"] or not request["authority"]["outcomeRefs"]:
            raise ValueError("simulation request requires adapter-validated outcome refs")


def main():
    contracts = ROOT / "Bridge/contracts/gate1"
    samples = ROOT / "Bridge/sample-data/gate1"
    load = lambda p: json.loads(p.read_text(encoding="utf8"))
    specs = {name: load(contracts / f"{name}.schema.json") for name in
             ["appearance-recipe", "vfx-parameters", "action-presentation-request"]}
    recipes = load(samples / "appearances.json")
    variants = load(samples / "aura-variants.json")
    requests = load(samples / "meditation-preview.json")
    checks = 0
    for group, name in [(recipes, "appearance-recipe"), (variants, "vfx-parameters")]:
        for value in group:
            validate(value, specs[name]); checks += 1
    for request in requests:
        check_request(request, specs["action-presentation-request"]); checks += 1
    assert len({x["actorRef"] for x in recipes}) == 3
    assert len({x["rigFamily"] for x in recipes}) == 1
    assert len({x["modules"]["body"] for x in recipes}) == 1
    assert len({(x["modules"]["head"], x["modules"]["hair"], x["modules"]["garment"], x["palette"]) for x in recipes}) == 3
    assert json.loads(json.dumps(recipes)) == recipes
    assert len({x["template"] for x in variants}) == 1
    assert len({tuple(x["color"]) for x in variants}) == 3
    assert [x["sequence"] for x in requests] == sorted({x["sequence"] for x in requests})
    assert len({x["requestId"] for x in requests}) == len(requests)
    checks += 9
    negatives = []
    for field, value in [("damage", 1), ("inventoryDelta", {}), ("worldTick", 10)]:
        bad = copy.deepcopy(requests[0]); bad[field] = value; negatives.append(bad)
    bad = copy.deepcopy(requests[0]); bad["kind"] = "execute_arbitrary_command"; negatives.append(bad)
    bad = copy.deepcopy(requests[0]); bad["authority"]["outcomeRefs"] = ["fake_success"]; negatives.append(bad)
    bad = copy.deepcopy(requests[0]); bad["authority"]["nonAuthoritative"] = False; negatives.append(bad)
    bad = copy.deepcopy(requests[0]); bad["visualDurationSeconds"] = -1; negatives.append(bad)
    bad = copy.deepcopy(requests[1]); bad["cosmeticCues"][0]["offsetSeconds"] = 999; negatives.append(bad)
    for bad in negatives:
        try:
            check_request(bad, specs["action-presentation-request"])
        except ValueError:
            checks += 1
        else:
            raise AssertionError("negative fixture was accepted")
    print(json.dumps({"result": "PASS", "checks": checks, "scope": "schemas and synthetic data only", "unrealBuild": "NOT_RUN", "rendering": "NOT_RUN"}))


if __name__ == "__main__":
    main()
