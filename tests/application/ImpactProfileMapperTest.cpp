#include <gtest/gtest.h>
#include "application/mappers/ImpactProfileMapper.hpp"

using namespace Application::Mappers;
using namespace SisterSTRATA::Observational::ImpactProfile::Domain;

TEST(ImpactProfileMapperTest, FormatOutputInPortuguese) {
    ReferenceFrame refFrame(
        ReferenceType::Historical, 
        "Baseline 2020", 
        "REF-2020"
    );

    StructuralDeviation structDev(0.5, 1.2, -50.0, "Alta Fragmentação");
    
    TemporalDeviationPattern tempPattern(
        DeviationTrend::Divergent, 
        0.1, 
        0.2, 
        "Divergência acelerada"
    );

    TrajectoryImpactProfile profile(
        "PROF-TEST",
        "OBS-2025",
        refFrame,
        structDev,
        tempPattern
    );

    std::string text = ImpactProfileMapper::toNaturalLanguage(profile);

    // Verify Portuguese Terms
    EXPECT_NE(text.find("ANÁLISE DE IMPACTO"), std::string::npos);
    EXPECT_NE(text.find("Referencial: Baseline 2020"), std::string::npos);
    EXPECT_NE(text.find("Histórico"), std::string::npos); // Enum mapping
    EXPECT_NE(text.find("Alta Fragmentação"), std::string::npos);
    EXPECT_NE(text.find("Divergente"), std::string::npos);
}
