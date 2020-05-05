import FWCore.ParameterSet.Config as cms

from RecoEcal.EgammaClusterProducers.particleFlowSuperClusterECALMustache_cfi import particleFlowSuperClusterECALMustache as _particleFlowSuperClusterECALMustache

# "Mustache" clustering
particleFlowSuperClusterECALHLT = _particleFlowSuperClusterECALMustache.clone()

# HLT regression setup
particleFlowSuperClusterECALHLT.regressionConfig = _particleFlowSuperClusterECALMustache.regressionConfig.clone(
    isHLT = cms.bool(True),
    eRecHitThreshold = cms.double(1.),
    regressionKeyEB  = cms.string('pfscecal_EBCorrection_online'),
    uncertaintyKeyEB = cms.string('pfscecal_EBUncertainty_online'),
    regressionKeyEE  = cms.string('pfscecal_EECorrection_online'),
    uncertaintyKeyEE = cms.string('pfscecal_EEUncertainty_online'),
    vertexCollection = cms.InputTag()
)
