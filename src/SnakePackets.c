#include "Snake.h"

void initPacket(GamePacket* gp, int32_t type) {

    gp->magic[0] = 'S';
    gp->magic[1] = 'K';

    gp->type = type;

}

bool isPacketValid(GamePacket* gp) {

    if(gp->magic[0] != 'S' || gp->magic[1] != 'K') {
        return false;
    }

    return true;

}
