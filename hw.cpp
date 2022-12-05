#define _CRT_SECURE_NO_WARNINGS
#include "snail.cpp"
#include "cow.cpp"
#include "_cow_supplement.cpp"
#include "jim.cpp"
#include "_cow_optimization.cpp"

enum Direction
{
    UP,
    DOWN,
    LEFT,
    RIGHT
};

struct Collision
{
    bool collided;
    Direction direction;
    vec2 difference;
};

Direction vector_direction(vec2 vec)
{
    vec2 compass[] = {
        V2(0., 1.),  // up
        V2(1., 0.),  // right
        V2(0., -1.), // down
        V2(-1., 0.)  // left
    };
    float max = 0.0f;
    unsigned int best_match = -1;
    for (unsigned int i = 0; i < 4; i++)
    {
        float dot_product = dot(normalized(vec), compass[i]);
        if (dot_product > max)
        {
            max = dot_product;
            best_match = i;
        }
    }
    return (Direction)best_match;
}

Collision check_ball_collision(vec2 collision[], vec2 ball_center)
{
    Collision result;

    double max_x = TINY;
    double min_x = HUGE;
    double max_y = TINY;
    double min_y = HUGE;

    for (int i = 0; i < 4; i++)
    {
        max_x = MAX(max_x, collision[i].x);
        min_x = MIN(min_x, collision[i].x);
        max_y = MAX(max_y, collision[i].y);
        min_y = MIN(min_y, collision[i].y);
    }

    vec2 object_center = {(max_x + min_x) / 2, (max_y + min_y) / 2};
    vec2 half_extents = {(max_x - min_x) / 2, (max_y - min_y) / 2};

    vec2 difference = ball_center - object_center;
    vec2 clamped = {CLAMP(difference.x, -half_extents.x, half_extents.x), CLAMP(difference.y, -half_extents.y, half_extents.y)};
    vec2 closest = object_center + clamped;

    difference = closest - ball_center;

    if (norm(difference) < 1)
    {
        result.collided = true;
        result.direction = vector_direction(difference);
        result.difference = difference;
    }
    else
    {
        result.collided = false;
    }

    return result;
}

void do_collision(Collision collision, vec2 vel, vec2 ball_center)
{
    Direction dir = collision.direction;
    vec2 diff = collision.difference;

    if (dir == LEFT || dir == RIGHT) // horizontal collision
    {
        vel.x = -vel.x; // reverse horizontal velocity

        // reposition ball to outside of object collided with
        // (since it would have gone slightly inside of the object
        // for collision detection)
        double reposition = 1 - abs(diff.x);

        if (dir == LEFT)
            ball_center.x += reposition; // move ball to right
        else
            ball_center.x -= reposition; // move ball to left;
    }
    else // vertical collision
    {
        vel.y = -vel.y; // reverse vertical velocity

        double reposition = 1 - abs(diff.y);

        if (dir == UP)
            ball_center.y -= reposition; // move ball back up
        else
            ball_center.y += reposition; // move ball back down
    }
}

void game()
{
    init();

    Camera2D camera = {40};

    int bg = 0;
    char *bgs[] = {"forest.jpeg", "city.png", "volcano.jpg", "moon.jpg", "vapor.jpg"};
    vec3 bg_vertex_positions[] = {
        {-40, -20, 0},
        {40, -20, 0},
        {40, 20, 0},
        {-40, 20, 0},
    };
    int3 triangle_indices[] = {{0, 1, 2}, {0, 2, 3}};
    vec2 vertex_texCoords[] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};

    vec2 initial_ball_center = {-20, -7};
    vec2 initial_shadow_center = {-20, -17};
    vec2 ball_center = initial_ball_center;
    vec2 shadow_center = initial_shadow_center;
    vec2 blacktop[] = {
        {-27, -19},
        {-18, -15},
        {19, -15},
        {28, -19},
    };
    vec2 blacktop_lines[] = {
        {25.5, -18},
        {-17.5, -18},
        {-13, -16},
        {21, -16},
    };
    vec3 blacktop_colors[] = {
        monokai.gray,
        V3(144, 76, 107) / 255.,
        V3(184, 29, 14) / 255.,
        V3(82, 69, 196) / 255.,
        V3(73, 209, 149) / 255.,
    };
    vec3 net_vertex_positions[] = {
        {15, -3, 0},
        {18, -3, 0},
        {18, 0, 0},
        {15, 0, 0},
    };
    vec2 front_rim_vertex_positions[] = {
        {15.25, 0.2},
        {15.25, -0.6},
        {14.75, -0.6},
        {14.75, 0.2},
    };
    vec2 back_rim_vertex_positions[] = {
        {18.25, 0.2},
        {18.25, -0.6},
        {17.75, -0.6},
        {17.75, 0.2},
    };
    vec2 front_net_vertex_positions[] = {
        {15.1, -0.6},
        {15.1, -3},
        {14.9, -3},
        {14.9, -0.6},
    };
    vec2 back_net_vertex_positions[] = {
        {18.1, -0.6},
        {18.1, -3},
        {17.9, -3},
        {17.9, -0.6},
    };
    vec2 backboard_vertex_positions[] = {
        {19.25, 5.25},
        {19, 5.25},
        {19, -2.25},
        {19.25, -2.25},
    };
    vec2 h_stand_vertex_positions[] = {
        {19.2, 2.25},
        {21.75, 2.25},
        {21.75, 1.5},
        {19.2, 1.5},
    };
    vec2 v_stand_vertex_positions[] = {
        {21.75, 1.5},
        {21, 1.5},
        {21, -17},
        {21.75, -17},
    };
    vec2 connect_rim_vertex_positions[] = {
        {18, -.15},
        {18, 0},
        {19, 0},
        {19, -.15},
    };

    vec2 *collision_quads[] = {
        front_rim_vertex_positions,
        back_rim_vertex_positions,
        front_net_vertex_positions,
        back_net_vertex_positions,
        backboard_vertex_positions,
        h_stand_vertex_positions,
        v_stand_vertex_positions,
        connect_rim_vertex_positions,
    };

    double acc = -9.81;
    vec2 vel;
    bool ball_shot = false;

    while (begin_frame())
    {
        camera_move(&camera, true, true);
        mat4 PV, V, M;
        { // set PV, V, M
            PV = camera_get_PV(&camera);
            V = Identity4x4;
            M = Identity4x4;
        }

        { // background
            fancy_draw(PV, V, M, 2, triangle_indices, 4, bg_vertex_positions, NULL, NULL, {}, vertex_texCoords, bgs[bg]);
        }

        { // blacktop
            basic_draw(QUADS, PV, 4, blacktop, blacktop_colors[bg]);
            basic_draw(LINE_STRIP, PV, 4, blacktop_lines, monokai.white);
        }

        { // net
            fancy_draw(PV, V, M, 2, triangle_indices, 4, net_vertex_positions, NULL, NULL, {}, vertex_texCoords, "net.png");
        }

        { // connect rim to backboard
            gl_PV(PV);
            gl_color(V3(253, 177, 89) / 255.);
            gl_begin(QUADS);
            for (int i = 0; i < 4; i++)
            {
                gl_vertex(connect_rim_vertex_positions[i]);
            }
            gl_end();
        }

        { // backboard
            gl_color(monokai.white);
            gl_begin(QUADS);
            for (int i = 0; i < 4; i++)
            {
                gl_vertex(backboard_vertex_positions[i]);
            }
            gl_end();
        }

        { // stand
            gl_begin(QUADS);
            for (int i = 0; i < 4; i++)
            {
                gl_vertex(h_stand_vertex_positions[i]);
            }
            gl_end();

            gl_begin(QUADS);
            for (int i = 0; i < 4; i++)
            {
                gl_vertex(v_stand_vertex_positions[i]);
            }
            gl_end();
        }

        { // ball
            vec3 ball_vertex_positions[] = {
                {ball_center.x - 1, ball_center.y - 1, 0},
                {ball_center.x + 1, ball_center.y - 1, 0},
                {ball_center.x + 1, ball_center.y + 1, 0},
                {ball_center.x - 1, ball_center.y + 1, 0},
            };
            vec3 ball_shadow_vertex_positions[] = {
                {shadow_center.x - 1, shadow_center.y - .5, 0},
                {shadow_center.x + 1, shadow_center.y - .5, 0},
                {shadow_center.x + 1, shadow_center.y + 1, 0},
                {shadow_center.x - 1, shadow_center.y + 1, 0},
            };

            vec2 start_drag;
            vec2 end_drag;

            if (input.mouse_left_pressed)
            {
                start_drag = input_get_mouse_position_in_world_coordinates(PV);
            }

            if (input.mouse_left_released)
            {
                end_drag = input_get_mouse_position_in_world_coordinates(PV);
                vel.x = start_drag.x - end_drag.x * 1.2;
                vel.y = start_drag.y - end_drag.y * 1.2;
                ball_shot = true;
            }

            if (ball_shot)
            {
                for (int i = 0; i < 8; i++)
                {
                    Collision collision = check_ball_collision(collision_quads[i], ball_center);
                    if (collision.collided)
                    {
                        /*
                        TODO:
                        doesn't work if i put into helper function... also bouncing is too violent
                        */

                        Direction dir = collision.direction;
                        vec2 diff = collision.difference;

                        if (dir == LEFT || dir == RIGHT) // horizontal collision
                        {
                            vel.x = -vel.x; // reverse horizontal velocity

                            // reposition ball to outside of object collided with
                            // (since it would have gone slightly inside of the object
                            // for collision detection)
                            // double reposition = 1 - abs(diff.x);

                            // if (dir == LEFT)
                            //     ball_center.x += reposition; // move ball to right
                            // else
                            //     ball_center.x -= reposition; // move ball to left;
                        }
                        else // vertical collision
                        {
                            vel.y = -vel.y; // reverse vertical velocity

                            // double reposition = 1 - abs(diff.y);

                            // if (dir == UP)
                            //     ball_center.y -= reposition; // move ball back up
                            // else
                            //     ball_center.y += reposition; // move ball back down
                        }
                    }
                }

                vel.y += acc / 20;
                ball_center += vel / 20;
                shadow_center.x += vel.x / 20;

                // when ball hits ground, reset it
                if (ball_center.y - 1 < -15)
                {
                    ball_shot = false;
                    ball_center = initial_ball_center;
                    shadow_center = initial_shadow_center;
                }
            }

            fancy_draw(PV, V, M, 2, triangle_indices, 4, ball_vertex_positions, NULL, NULL, {}, vertex_texCoords, "basketball.png");

            // ball shadow
            fancy_draw(PV, V, M, 2, triangle_indices, 4, ball_shadow_vertex_positions, NULL, NULL, {}, vertex_texCoords, "basketball.png", true);
        }

        imgui_slider("background", &bg, 0, 4, 'j', 'k', true);
    }
}

int main()
{
    game();
    return 0;
}
