# Bridge Contracts

This directory will contain transport-neutral contracts between the authoritative `tu-ti-n` simulation and the Unreal presentation layer.

Rules:

- no Unreal-specific type names in core contracts;
- no direct references to TypeScript implementation classes;
- contracts must be versioned before live integration;
- sample data is allowed before transport is chosen;
- authoritative formulas never live here.

Initial contract families planned for the vertical slice:

- `WorldSnapshot`
- `PlayerIntent`
- `ActionOutcome`
- `PresentationEvent`

No transport is selected yet. HTTP, local IPC, embedded runtime, sockets, or another mechanism will be evaluated only after the Unreal slice can run against mock data.
