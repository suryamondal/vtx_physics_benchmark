#include "Constants.hh" // Own include

const TString CommonCuts =
  "Dst_M > 1.97 && Dst_M < 2.04" // Asymmetric, tuned to simulation
  "&& Dst_M_preFit > 1.6 && Dst_M_preFit < 2.4" // Pre-fit selection simulation
  "&& D0_M > 1.82 && D0_M < 1.89" // Asymmetric, tuned to simulation
  "&& D0_M_preFit > 1.4 && D0_M_preFit < 2.2" // Pre-fit selection simulation
  "&& massDiff > 0.144 && massDiff < 0.147" // Tuned to simulation
  "&& massDiffPreFit < 0.2" // Pre-fit selection simulation
  "&& K_dr < 2 && K_dz > -2 && K_dz < 2" // Pre-fit selection simulation
  "&& pisoft_dr < 2 && pisoft_dz > -2 && pisoft_dz < 2" // Pre-fit selection simulation
  "&& K_nVXDHits > 0 && pisoft_nVXDHits > 0" // VXD = PXD+SVD+VTX, pre-fit selection simulation
  "&& Dst_p_CMS < 2.45"; // From momentum conservation

const TString KpiCuts =
  "pi_dr < 2 && pi_dz > -2 && pi_dz < 2" // Pre-fit selection simulation
  "&& pi_nVXDHits > 0"; // VXD = PXD+SVD+VTX, pre-fit selection simulation

const TString K3piCuts =
  "pi1_dr < 2 && pi1_dz > -2 && pi1_dz < 2" // Pre-fit selection simulation
  "&& pi2_dr < 2 && pi2_dz > -2 && pi2_dz < 2" // Pre-fit selection simulation
  "&& pi3_dr < 2 && pi3_dz > -2 && pi3_dz < 2" // Pre-fit selection simulation
  "&& pi1_nVXDHits > 0 && pi2_nVXDHits > 0 && pi3_nVXDHits > 0" // VXD = PXD+SVD+VTX, pre-fit selection simulation
  "&& !(K_pionID > 0.05) && !(pi1_kaonID > 0.97) && !(pi2_kaonID > 0.97) && !(pi3_kaonID > 0.97)" // NOT in the steering files, doesn't cut nans
  "&& !(K_kaonID < 0.05) && !(pi1_pionID < 0.03) && !(pi2_pionID < 0.03) && !(pi3_pionID < 0.03)" // Same as above
  ;//"&& Dst_M > 2 && Dst_M < 2.02" // Harder cuts on masses for K3pi
  //"&& D0_M > 1.854 && D0_M < 1.874" // Harder cuts on masses for K3pi
  //"&& massDiff > 0.1442 && massDiff < 0.1466"; // Harder cuts on masses for K3pi

/* The "harder cuts on masses for K3pi"
 *  - reduce the signal efficiency from 45-50% to 35-40%
 *  - improve the SNR from 2.7-3 to 7.3-7.4
 *  - these two figures come from no-bkg samples
 * Is it worth using them?
 */
