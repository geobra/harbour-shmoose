#include "Omemo.h"

#include "libomemo.h"
#include "libomemo_crypto.h"
#include "libomemo_storage.h"

#include "axc.h"
#include "axc_store.h"

Omemo::Omemo(QObject *parent) : QObject(parent)
{

}


void Omemo::omemo_default_crypto_init(void)
{
  (void) gcry_check_version((void *) 0);
  gcry_control(GCRYCTL_SUSPEND_SECMEM_WARN);
  gcry_control (GCRYCTL_INIT_SECMEM, 16384, 0);
  gcry_control (GCRYCTL_RESUME_SECMEM_WARN);
  gcry_control(GCRYCTL_USE_SECURE_RNDPOOL);
  gcry_control (GCRYCTL_INITIALIZATION_FINISHED, 0);
}
