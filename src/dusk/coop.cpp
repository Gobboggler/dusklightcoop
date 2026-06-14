#include "dusk/coop.hpp"
#include "dusk/settings.h"
#include "d/d_com_inf_game.h"
#include "d/actor/d_a_alink.h"
#include "f_op/f_op_actor_mng.h"

namespace dusk {

void spawnCoopPlayers() {
    int count = dusk::getSettings().backend.splitScreenPlayerCount.getValue();
    if (count <= 1) return;
    if (count > 4) count = 4;

    for (int i = 1; i < count; i++) {
        daAlink_c* mainPlayer = (daAlink_c*)dComIfGp_getPlayer(0);
        if (!mainPlayer) continue;

        cXyz pos = mainPlayer->current.pos;
        csXyz angle(mainPlayer->shape_angle.x, mainPlayer->shape_angle.y, mainPlayer->shape_angle.z);
        int roomNo = fopAcM_GetRoomNo(mainPlayer);

        // Bit 8 set = coop player, low 8 bits = player index
        u32 param = (1 << 8) | i;
        fopAcM_fastCreate(fpcNm_ALINK_e, param, &pos, roomNo, &angle, NULL, -1, NULL, NULL);
    }
}

} // namespace dusk
