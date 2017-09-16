
#ifndef DESIGNWARE_H

#define DESIGNWARE_H

struct xEMACInterface {
};

typedef struct xEMACInterface EMACInterface_t;

void emacps_check_rx( EMACInterface_t *pxEMACif );
void emacps_check_tx( EMACInterface_t *pxEMACif );
void emacps_check_errors( EMACInterface_t *pxEMACif );

uint32_t Phy_Setup( EMACInterface_t *pxEMACif );

#endif
