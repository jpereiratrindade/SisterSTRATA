# Exception Boundary Policy

Status: Active  
Owner: Architecture Governance Lead  
Review cadence: Per release cycle

## 1. Purpose

Define where broad exception shields are allowed and how failures must remain observable.

## 2. Rule

`catch(...)` is forbidden in core/domain and application service logic.

It is allowed only at process/runtime boundaries when all conditions are true:

1. the boundary cannot safely propagate unknown exception types,
2. structured telemetry is emitted (`ERROR` level at minimum),
3. the execution path fails fast or returns an explicit failure code.

## 3. Allowed Boundary Zones

- Process entrypoints (for example `src/main.cpp`)
- External callback boundaries (for example UI/engine callback adapters)

## 4. Disallowed Zones

- `src/core/domain/**`
- `src/application/services/**`
- `src/infrastructure/**` business flow handlers

## 5. Current STRATA Mapping

- `src/main.cpp`: boundary shield, emits fatal stderr message, exits with non-zero code.
- `src/world3d/Engine.cpp` callback wrapper: boundary shield, emits callback failure telemetry.

## 6. F2 Follow-up

Automate this policy with static checks (grep/AST) in CI so new forbidden `catch(...)` usage fails merge.
