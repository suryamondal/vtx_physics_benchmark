#include "Constants.hh" // Own include

const TString CommonCuts =
  "Dst_M > 2 && Dst_M < 2.02" // In steering file is [1.8,2.2]
  "&& Dst_M_preFit > 1.6 && Dst_M_preFit < 2.4" // Identical to steering file
  "&& D0_M > 1.854 && D0_M < 1.874" // In steering file is [1.66,2.06]
  "&& D0_M_preFit > 1.4 && D0_M_preFit < 2.2" // Identical to steering file
  "&& massDiff > 0.1442 && massDiff < 0.1466" // In the steering file it's < 0.16
  "&& massDiffPreFit < 0.2" // Identical to steering file
  "&& K_dr < 2 && K_dz > -2 && K_dz < 2" // Identical to steering file
  "&& pisoft_dr < 2 && pisoft_dz > -2 && pisoft_dz < 2" // Identical to steering file
  "&& K_nVXDHits > 0 && pisoft_nVXDHits > 0" // VXD = PXD+SVD+VTX, identical to steering file
  "&& Dst_p_CMS < 2.45" // This comes from conservation of momentum, and is NOT in the steering file
  "&& !(K_pionID > 0.3) && !(K_kaonID < 0.02)" // This is NOT in the steering file, and there are some nans
  "&& !(pisoft_kaonID > 0.3) && !(pisoft_pionID < 0.02)"; // This is NOT in the steering file, and there are some nans

const TString KpiCuts =
  "pi_dr < 2 && pi_dz > -2 && pi_dz < 2" // Identical to steering file
  "&& pi_nVXDHits > 0" // VXD = PXD+SVD+VTX, identical to steering file
  "&& !(pi_kaonID > 0.98)"; // This is NOT in the steering file, and there are some nans

const TString K3piCuts =
  "pi1_dr < 2 && pi1_dz > -2 && pi1_dz < 2" // Identical to steering file
  "&& pi2_dr < 2 && pi2_dz > -2 && pi2_dz < 2" // Identical to steering file
  "&& pi3_dr < 2 && pi3_dz > -2 && pi3_dz < 2" // Identical to steering file
  "&& pi1_nVXDHits > 0 && pi2_nVXDHits > 0 && pi3_nVXDHits > 0" // VXD = PXD+SVD+VTX, identical to steering file
  "&& !(pi1_kaonID > 0.98) && !(pi2_kaonID > 0.98) && !(pi3_kaonID > 0.98)"; // This is NOT in the steering file, and there are some nans
