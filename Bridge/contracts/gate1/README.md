# Gate 1 presentation contracts

Version 1, transport-neutral, presentation-only. No live simulation transport is implemented. Fixtures are synthetic offline mocks and do not claim accepted cultivation outcomes. Module keys identify future target assets; no corresponding licensed mesh/rig/clip is supplied.

Validate with `python Scripts/validate_gate1_contracts.py` (standard library). The checker supports the schema constructs used here and adds authority, sequence and fixture invariants. It is a fixture/contract regression check, not a production JSON decoder or Unreal build test. Use a full Draft 2020-12 validator at external adapter admission.

Consumers must authenticate/validate source context, deduplicate request/cue IDs, reject stale revisions and prevent presentation state from writing authoritative domains. Never execute strings in these envelopes. Time is visual seconds only; simulation time is absent.
