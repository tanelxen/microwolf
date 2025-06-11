//
//  PlayerMovement.cpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 02.11.24.
//

#include "PlayerMovement.h"

#include <GLFW/glfw3.h>
#include "Q3BspCollision.h"

static constexpr float cl_forwardspeed = 400.0;
static constexpr float cl_sidespeed = 350.0;
static constexpr float cl_movement_accelerate = 15.0;
static constexpr float cl_movement_airaccelerate = 7.0;
static constexpr float cl_movement_friction = 8.0;
static constexpr float sv_gravity = 800.0;
static constexpr float sv_max_speed = 320.0;
static constexpr float cl_stop_speed = 200.0;
static constexpr float cpm_air_stop_acceleration = 2.5;
static constexpr float cpm_air_control_amount = 150.0;
static constexpr float cpm_strafe_acceleration = 70.0;
static constexpr float cpm_wish_speed = 30.0;

static constexpr int MAX_CLIP_PLANES = 5;
static constexpr float OVERCLIP = 1.001;
static constexpr float STEPSIZE = 18;

static glm::vec3 player_mins = { -15, -15, -24 };
static glm::vec3 player_maxs = { 15, 15, 32 };

enum movement_bits
{
    MOVEMENT_JUMP = 1<<1,
    MOVEMENT_JUMP_THIS_FRAME = 1<<2,
    MOVEMENT_JUMPING = 1<<3,
    LAST_MOVEMENT_BIT
};

static bool noclip = false;

void clip_velocity(const glm::vec3& in, const glm::vec3& normal, glm::vec3& out, float overbounce);


PlayerMovement::PlayerMovement(Q3BspCollision *collision) : m_pCollision(collision)
{
    velocity = {0, 0, 0};
}

void PlayerMovement::setTransform(const glm::vec3 &position, const glm::vec3 &forward, const glm::vec3 &right, const glm::vec3 &up)
{
    m_position = position;
    m_forward = glm::normalize(forward);
    m_right = glm::normalize(right);
    m_up = up;
}

void PlayerMovement::setInputMovement(int forward, int right, bool jump)
{
    m_forwardmove = forward;
    m_rightmove = right;
    
    
    if (jump)
    {
        m_movementBits |= MOVEMENT_JUMP;
    }
    else
    {
        m_movementBits &= ~MOVEMENT_JUMP;
    }
}

void PlayerMovement::update(float dt)
{
    if (!noclip)
    {
        trace_ground();
        apply_inputs(dt);
        
        bool gravity = (m_movementBits & MOVEMENT_JUMPING) != 0;
        step_slide(gravity, dt);
    }
    else
    {
        glm::vec3 direction = {0, 0, 0};
        direction += m_forward * m_forwardmove * cl_forwardspeed;
        direction += m_right * m_rightmove * cl_forwardspeed;
        
        m_position += direction * dt;
    }

    m_movementBits &= ~MOVEMENT_JUMP_THIS_FRAME;
    
    m_forwardmove = 0;
    m_rightmove = 0;
}

void PlayerMovement::trace_ground()
{
    glm::vec3 point = m_position;
    point.z -= 0.25;
    
    HitResult result;
    m_pCollision->trace(result, m_position, point, player_mins, player_maxs);

    if (result.fraction == 1 || (m_movementBits & MOVEMENT_JUMP_THIS_FRAME))
    {
        m_movementBits |= MOVEMENT_JUMPING;
        ground_normal = {0, 0, 0};
        surfaceFlags = 0;
        isGrounded = false;
    }
    else
    {
        m_movementBits &= ~MOVEMENT_JUMPING;
        ground_normal = result.normal;
        surfaceFlags = result.surfaceFlags;
        isGrounded = true;
    }
}

void PlayerMovement::apply_inputs(float dt)
{
    glm::vec3 forward = m_forward;
    glm::vec3 right = m_right;
    
    forward.z = 0;
    right.z = 0;
    
    glm::vec3 direction = {0, 0, 0};
    direction += forward * m_forwardmove * cl_forwardspeed;
    direction += right * m_rightmove * cl_sidespeed;

    /* movement */
    float wishspeed = glm::length(direction);
    wishspeed = fmin(wishspeed, sv_max_speed);
    
    if (wishspeed >= 0.0001f) {
        direction /= wishspeed;
    }

    apply_jump();
    apply_friction(dt);

    float selected_acceleration = cl_movement_accelerate;
    float base_wishspeed = wishspeed;

    // cpm air acceleration
    if (noclip || (m_movementBits & MOVEMENT_JUMPING) || (m_movementBits & MOVEMENT_JUMP_THIS_FRAME))
    {
        if (glm::dot(velocity, direction) < 0) {
            selected_acceleration = cpm_air_stop_acceleration;
        } else {
            selected_acceleration = cl_movement_airaccelerate;
        }

        if (m_rightmove != 0 && m_forwardmove == 0) {
            wishspeed = fmin(wishspeed, cpm_wish_speed);
            selected_acceleration = cpm_strafe_acceleration;
        }
    }

    apply_acceleration(direction, wishspeed, selected_acceleration, dt);
    apply_air_control(direction, base_wishspeed, dt);
}

void PlayerMovement::apply_air_control(const glm::vec3& direction, float wishspeed, float dt)
{
    if (m_forwardmove == 0 || wishspeed == 0) return;

    float falling_speed = velocity.z;
    velocity.z = 0;
    
    float speed = sqrt(glm::dot(velocity, velocity));
    
    if (speed >= 0.0001f) {
        velocity /= speed;
    }
    
    float dot = glm::dot(velocity, direction);
    
    if (dot > 0)
    {
        velocity *= speed;
        velocity = glm::normalize(velocity);
    }

    velocity *= speed;
    velocity.z = falling_speed;
}

void PlayerMovement::apply_acceleration(const glm::vec3& direction, float wishspeed, float acceleration, float dt)
{
    if (!noclip && (m_movementBits & MOVEMENT_JUMPING)) {
        wishspeed = fmin(cpm_wish_speed, wishspeed);
    }

    float cur_speed = glm::dot(velocity, direction);
    float add_speed = wishspeed - cur_speed;

    if (add_speed <= 0) return;

    float accel_speed = acceleration * dt * wishspeed;
    accel_speed = fmin(accel_speed, add_speed);
    
    velocity += direction * accel_speed;
}

void PlayerMovement::apply_friction(float dt)
{
    if (!noclip)
    {
        if ((m_movementBits & MOVEMENT_JUMPING) || (m_movementBits & MOVEMENT_JUMP_THIS_FRAME)) return;
    }

    float speed = sqrt(glm::dot(velocity, velocity));
    
    if (speed < 1) {
        velocity.x = 0;
        velocity.y = 0;
        return;
    }

    float control = speed < cl_stop_speed ? cl_stop_speed : speed;
    float new_speed = speed - control * cl_movement_friction * dt;

    velocity *= fmax(0, new_speed) / speed;
}

void PlayerMovement::apply_jump()
{
    if (!(m_movementBits & MOVEMENT_JUMP)) return;
    if ((m_movementBits & MOVEMENT_JUMPING) && !noclip) return;
    
    m_movementBits |= MOVEMENT_JUMP_THIS_FRAME;
    m_movementBits &= ~MOVEMENT_JUMP;
    
    velocity.z = 270;
}

void PlayerMovement::step_slide(bool gravity, float dt)
{
    glm::vec3 start_o = m_position;
    glm::vec3 start_v = velocity;
    
    if (!slide(gravity, dt))
    {
        // we got exactly where we wanted to go first try
        return;
    }
    
    glm::vec3 down = start_o;
    down.z -= STEPSIZE;
    
    HitResult result;
    m_pCollision->trace(result, start_o, down, player_mins, player_maxs);

    glm::vec3 up = {0, 0, 1};
    
    // never step up when you still have up velocity
    if (velocity.z > 0 && (result.fraction == 1.0 || glm::dot(result.normal, up) < 0.7))
    {
        return;
    }

    up = start_o;
    up.z += STEPSIZE;
    
    // test the player position if they were a stepheight higher
    m_pCollision->trace(result, up, up, player_mins, player_maxs);
    
    if (result.allsolid)
    {
        // can't step up
        return;
    }

    // try slidemove from this position
    m_position = up;
    velocity = start_v;
    
    slide(gravity, dt);

    // push down the final amount
    down = m_position;
    down.z -= STEPSIZE;
    
    m_pCollision->trace(result, m_position, down, player_mins, player_maxs);
    
    if (!result.allsolid)
    {
        m_position = result.endpos;
    }
    
    if (result.fraction < 1.0)
    {
        clip_velocity(velocity, result.normal, velocity, OVERCLIP);
    }
}

bool PlayerMovement::slide(bool gravity, float dt)
{
    glm::vec3 end_velocity;
    glm::vec3 planes[MAX_CLIP_PLANES];

    if (gravity)
    {
        end_velocity = velocity;
        end_velocity.z -= sv_gravity * dt;

        /*
         * not 100% sure why this is necessary, maybe to avoid tunneling
         * through the floor when really close to it
         */

        velocity.z = (end_velocity.z + velocity.z) * 0.5f;

        /* slide against floor */
        if (isGrounded) {
            clip_velocity(velocity, ground_normal, velocity, OVERCLIP);
        }
    }
    
    int n_planes = 0;

    if (isGrounded) {
        planes[n_planes] = ground_normal;
        ++n_planes;
    }
    
    planes[n_planes] = glm::normalize(velocity);
    ++n_planes;
    
    int n_bumps;
    float time_left = dt;

    for (n_bumps = 0; n_bumps < 4; ++n_bumps)
    {
        /* calculate future position and attempt the move */
        HitResult work;
        glm::vec3 end = m_position + velocity * time_left;
        m_pCollision->trace(work, m_position, end, player_mins, player_maxs);
        
//        printf("(fraction = %.1f) (allsolid = %i) \n", work.fraction, work.allsolid);
        
        if (work.allsolid)
        {
            // entity is completely trapped in another solid
            // don't build up falling damage, but allow sideways acceleration
            velocity.z = 0;
            return false;
        }

        if (work.fraction > 0) {
            m_position = work.endpos;
        }

        /* if nothing blocked us we are done */
        if (work.fraction == 1) {
            break;
        }

        time_left -= time_left * work.fraction;

        if (n_planes >= MAX_CLIP_PLANES) {
            velocity = {0, 0, 0};
            return false;
        }

        /*
         * if it's a plane we hit before, nudge velocity along it
         * to prevent epsilon issues and dont re-test it
         */
        
        int i;

        for (i = 0; i < n_planes; ++i)
        {
            if (glm::dot(work.normal, planes[i]) > 0.99) {
                velocity += work.normal;
                break;
            }
        }

        if (i < n_planes) {
            continue;
        }

        /*
         * entirely new plane, add it and clip velocity against all
         * planes that the move interacts with
         */
        
        planes[n_planes] = work.normal;
        ++n_planes;

        for (i = 0; i < n_planes; ++i)
        {
            if (glm::dot(velocity, planes[i]) >= 0.1) {
                continue;
            }

            glm::vec3 clipped;
            glm::vec3 end_clipped;
            
            clip_velocity(velocity, planes[i], clipped, OVERCLIP);
            clip_velocity(end_velocity, planes[i], end_clipped, OVERCLIP);

            /*
             * if the clipped move still hits another plane, slide along
             * the line where the two planes meet (cross product) with the
             * un-clipped velocity
             *
             * TODO: reduce nesting in here
             */

            for (int j = 0; j < n_planes; ++j)
            {
                if (j == i) {
                    continue;
                }

                if (glm::dot(clipped, planes[j]) >= 0.1) {
                    continue;
                }

                clip_velocity(clipped, planes[j], clipped, OVERCLIP);
                clip_velocity(end_clipped, planes[j], end_clipped, OVERCLIP);

                if (glm::dot(clipped, planes[i]) >= 0) {
                    /* goes back into the first plane */
                    continue;
                }
                
                glm::vec3 dir = glm::normalize(glm::cross(planes[i], planes[j]));

                clipped = dir * glm::dot(dir, velocity);
                end_clipped = dir * glm::dot(dir, end_velocity);


                /* if we still hit a plane, just give up and dead stop */

                for (int k = 0; k < n_planes; ++k)
                {
                    if (k == j || k == i) {
                        continue;
                    }

                    if (glm::dot(clipped, planes[k]) >= 0.1) {
                        continue;
                    }

                    velocity = {0, 0, 0};
                    return false;
                }
            }

            /* resolved all collisions for this move */
            velocity = clipped;
            end_velocity = end_clipped;
            break;
        }
    }

    if (gravity) {
        velocity = end_velocity;
    }
    
    return n_bumps > 0;
}


void clip_velocity(const glm::vec3& in, const glm::vec3& normal, glm::vec3& out, float overbounce)
{
    float backoff = glm::dot(in, normal);

    if (backoff < 0) {
        backoff *= overbounce;
    } else {
        backoff /= overbounce;
    }

    out = in - normal * backoff;
}
