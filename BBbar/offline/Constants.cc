#include "Constants.hh" // Own include

const TString CommonCuts =
  "Dst_M > 1.8 && Dst_M < 2.2 && Dst_M_preFit > 1.6 && Dst_M_preFit < 2.4"
  "&& D0_M > 1.66 && D0_M < 2.06 && D0_M_preFit > 1.4 && D0_M_preFit < 2.2"
  "&& massDiffPreFit < 0.2 && massDiff < 0.151" // In the steering file it's < 0.16
  "&& K_dr < 2 && K_dz > -2 && K_dz < 2"
  "&& pisoft_dr < 2 && pisoft_dz > -2 && pisoft_dz < 2"
  "&& K_nVXDHits > 0 && pisoft_nVXDHits > 0" // VXD = PXD+SVD+VTX
  "&& Dst_p_CMS < 2.45" // This comes from conservation of momentum
  "&& pisoft_pionID > 0.1";
  //"&& !(pisoft_diffKaonPionID > 0.2)"; // This is NOT in the steering file, and there are some nans

const TString KpiCuts =
  "pi_dr < 2 && pi_dz > -2 && pi_dz < 2"
  "&& pi_nVXDHits > 0"; // VXD = PXD+SVD+VTX

const TString K3piCuts =
  "pi1_dr < 2 && pi1_dz > -2 && pi1_dz < 2"
  "&& pi2_dr < 2 && pi2_dz > -2 && pi2_dz < 2"
  "&& pi3_dr < 2 && pi3_dz > -2 && pi3_dz < 2"
  "&& pi1_nVXDHits > 0 && pi2_nVXDHits > 0 && pi3_nVXDHits > 0"; // VXD = PXD+SVD+VTX
