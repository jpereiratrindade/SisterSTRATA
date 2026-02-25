#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="${1:-$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)}"
cd "${ROOT_DIR}"

JOBS="${STRATA_JOBS:-$(nproc)}"

echo "[f2-suite] configure: cmake --preset headless"
cmake --preset headless

echo "[f2-suite] build: cmake --build --preset headless -j${JOBS}"
cmake --build --preset headless -j"${JOBS}"

echo "[f2-suite] run application gates"
ctest --test-dir build/headless --output-on-failure -R \
    "ApplicationMapper_MembraneContract|ApplicationMapper_CrossContextIsolation|ApplicationMapper_InfrastructureResilience"

echo "[f2-suite] run core scientific gates"
ctest --test-dir build/headless --output-on-failure -R \
    "CoreWorkspaceTest|CorePatchAnalysisTest|CoreSoilRasterizerTest|CorePatchTrajectoryTest|CoreTrajectoryLODTest|CoreVegVectorTest|CoreSlopeFixTest|CoreInfrastructureTest|CoreEnergyAllocationPolicyTest|CoreHydroDomainTest|CoreEnvironmentControllerDeterminismTest|CoreHypothesisIdDeterminismTest"

echo "[f2-suite] run governance guards"
ctest --test-dir build/headless --output-on-failure -R \
    "MembraneDependencyGuard|CoreDomainBoundaryGuard|CoreDeterminismPrimitiveGuard|ExceptionBoundaryGuard|GitArtifactHygieneGuard|VersionAlignmentGuard"

echo "[f2-suite] completed successfully"
