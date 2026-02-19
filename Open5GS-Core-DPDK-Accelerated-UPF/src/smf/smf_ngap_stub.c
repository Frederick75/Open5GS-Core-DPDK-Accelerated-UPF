#include "common/log.h"
// Placeholder for NGAP/N11 integration.
// In a full Open5GS integration, UE-initiated PDU session requests arrive via AMF (N11 / SBI).
// This reference project provides a built-in demo flow triggered by CLI flag.
void smf_ngap_stub_note(void){
  LOGD("NGAP/SBI stub: integrate with AMF/NRF in a full system.");
}
