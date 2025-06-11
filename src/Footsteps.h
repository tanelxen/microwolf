//
//  Footsteps.hpp
//  wolf
//
//  Created by Fedor Artemenkov on 10.06.25.
//

#pragma once

struct Footsteps
{
    void init();
    void update(float dt, struct PlayerMovement* movement);
};
