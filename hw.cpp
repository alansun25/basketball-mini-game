#define _CRT_SECURE_NO_WARNINGS
#include "snail.cpp"
#include "cow.cpp"
#include "_cow_supplement.cpp"
#include "jim.cpp"
#include "_cow_optimization.cpp"

const double BALL_RADIUS = 1;

enum Direction
{
    UP,
    DOWN,
    LEFT,
    RIGHT
};

enum Orientation
{
    VERTICAL,
    HORIZONTAL
};

struct Collision
{
    bool collided;
    vec2 difference;
    Orientation orientation;
    Direction direction;
};

Collision check_ball_collision(vec2 collision[], vec2 ball_center, bool is_rim = false)
{
    Collision result;

    double max_x = -HUGE;
    double min_x = HUGE;
    double max_y = -HUGE;
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

    // if collided
    if (norm(difference) <= BALL_RADIUS)
    {
        result.collided = true;
        result.difference = difference;
        if (is_rim)
        {
            if (ball_center.y >= max_y || ball_center.y <= min_y)
            {
                result.orientation = VERTICAL;
                if (ball_center.y >= max_y)
                {
                    result.direction = DOWN;
                }
                else
                {
                    result.direction = UP;
                }
            }
            else if (ball_center.x >= max_x || ball_center.x <= min_x)
            {
                result.orientation = HORIZONTAL;
                if (ball_center.y >= max_x)
                {
                    result.direction = LEFT;
                }
                else
                {
                    result.direction = RIGHT;
                }
            }
        }
        else
        {
            if (ball_center.x >= max_x || ball_center.x <= min_x)
            {
                result.orientation = HORIZONTAL;
                if (ball_center.y >= max_x)
                {
                    result.direction = LEFT;
                }
                else
                {
                    result.direction = RIGHT;
                }
            }
            else if (ball_center.y >= max_y || ball_center.y <= min_y)
            {
                result.orientation = VERTICAL;
                if (ball_center.y >= max_y)
                {
                    result.direction = DOWN;
                }
                else
                {
                    result.direction = UP;
                }
            }
        }
    }
    else
    {
        result.collided = false;
    }

    return result;
}

void game()
{
    init();

    Camera2D camera = {40};

    int bg = 0;
    char *bgs[] = {"forest.jpeg", "city.png", "volcano.jpg", "moon.jpg", "vapor.jpg"};
    vec3 bg_vp[] = {
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
    vec3 net_vp[] = {
        {15, -3, 0},
        {18, -3, 0},
        {18, 0, 0},
        {15, 0, 0},
    };
    vec2 connect_rim_display_vp[] = {
        {18, -.15},
        {18, 0},
        {19, 0},
        {19, -.15},
    };

    vec2 ground[] = {
        {-40, -17},
        {40, -17},
        {40, -20},
        {-40, -20},
    };
    vec2 front_rim_vp[] = {
        {15.25, -0.6},
        {15.25, 0},
        {14.75, 0},
        {14.75, -0.6},
    };
    vec2 back_rim_vp[] = {
        {18.25, -0.6},
        {18.25, 0},
        {17.75, 0},
        {17.75, -0.6},
    };
    vec2 front_net_vp[] = {
        {15.25, -3},
        {15.25, -0.6},
        {14.75, -0.6},
        {14.75, -3},
    };
    vec2 back_net_vp[] = {
        {18.25, -3},
        {18.25, -0.6},
        {17.75, -0.6},
        {17.75, -3},
    };
    vec2 backboard_vp[] = {
        {19.25, 5.25},
        {19, 5.25},
        {19, -2.25},
        {19.25, -2.25},
    };
    vec2 h_stand_vp[] = {
        {19.2, 2.25},
        {21.75, 2.25},
        {21.75, 1.5},
        {19.2, 1.5},
    };
    vec2 v_stand_vp[] = {
        {21.75, 1.5},
        {21, 1.5},
        {21, -17},
        {21.75, -17},
    };
    vec2 connect_rim_vp[] = {
        {18, -.5},
        {18, 0},
        {19, 0},
        {19, -.5},
    };

    vec2 *collision_quads[] = {
        ground,
        front_rim_vp,
        back_rim_vp,
        backboard_vp,
        h_stand_vp,
        v_stand_vp,
        connect_rim_vp,
        front_net_vp,
        back_net_vp,
    };
    vec2 bottom_net[] = {
        {15.5, -3},
        {17.5, -3},
        {17.5, -3.01},
        {15.5, -3.01}};

    double acc = -9.81;
    vec2 vel;
    bool ball_shot = false;
    int shots_made = 0;
    bool ball_in_hoop = false;

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
            fancy_draw(PV, V, M, 2, triangle_indices, 4, bg_vp, NULL, NULL, {}, vertex_texCoords, bgs[bg]);
        }

        { // blacktop
            basic_draw(QUADS, PV, 4, blacktop, blacktop_colors[bg]);
            basic_draw(LINE_STRIP, PV, 4, blacktop_lines, monokai.white);
        }

        { // connect rim to backboard
            gl_PV(PV);
            gl_color(V3(253, 177, 89) / 255.);
            gl_begin(QUADS);
            for (int i = 0; i < 4; i++)
            {
                gl_vertex(connect_rim_display_vp[i]);
            }
            gl_end();
        }

        { // backboard
            gl_color(monokai.white);
            gl_begin(QUADS);
            for (int i = 0; i < 4; i++)
            {
                gl_vertex(backboard_vp[i]);
            }
            gl_end();
        }

        { // stand
            gl_begin(QUADS);
            for (int i = 0; i < 4; i++)
            {
                gl_vertex(h_stand_vp[i]);
            }
            gl_end();

            gl_begin(QUADS);
            for (int i = 0; i < 4; i++)
            {
                gl_vertex(v_stand_vp[i]);
            }
            gl_end();
        }

        { // ball
            vec3 ball_vp[] = {
                {ball_center.x - BALL_RADIUS, ball_center.y - BALL_RADIUS, 0},
                {ball_center.x + BALL_RADIUS, ball_center.y - BALL_RADIUS, 0},
                {ball_center.x + BALL_RADIUS, ball_center.y + BALL_RADIUS, 0},
                {ball_center.x - BALL_RADIUS, ball_center.y + BALL_RADIUS, 0},
            };
            vec3 ball_shadow_vp[] = {
                {shadow_center.x - BALL_RADIUS, shadow_center.y - BALL_RADIUS / 2, 0},
                {shadow_center.x + BALL_RADIUS, shadow_center.y - BALL_RADIUS / 2, 0},
                {shadow_center.x + BALL_RADIUS, shadow_center.y + BALL_RADIUS, 0},
                {shadow_center.x - BALL_RADIUS, shadow_center.y + BALL_RADIUS, 0},
            };

            // ball shadow
            fancy_draw(PV, V, M, 2, triangle_indices, 4, ball_shadow_vp, NULL, NULL, {}, vertex_texCoords, "basketball.png", true);

            static vec2 start_drag;
            static vec2 end_drag;
            static vec2 potential_drag;
            vec2 potential_ball_center = ball_center;
            vec2 potential_vel;

            if (input.mouse_left_pressed)
            {
                start_drag = input_get_mouse_position_in_world_coordinates(PV);
            }

            if (input.mouse_left_held)
            {
                potential_drag = input_get_mouse_position_in_world_coordinates(PV);
                potential_vel = start_drag - potential_drag * 1.2;

                // draw trajectory arc
                gl_color(monokai.orange);
                gl_begin(POINTS);
                for (int i = 0; i < 70; i++)
                {
                    potential_vel.y += acc / 20;
                    potential_ball_center += potential_vel / 20;
                    gl_vertex(potential_ball_center);
                }
                gl_end();
            }

            if (input.mouse_left_released)
            {
                end_drag = input_get_mouse_position_in_world_coordinates(PV);
                vel = start_drag - end_drag * 1.2;
                ball_shot = true;
            }

            if (ball_shot)
            {
                vel.y += acc / 20;

                for (int i = 0; i < NELEMS(collision_quads); i++)
                {
                    bool is_rim = i == 1 || i == 2 || i == 6;
                    Collision collision = check_ball_collision(collision_quads[i], ball_center, is_rim);
                    if (collision.collided)
                    {
                        Orientation ori = collision.orientation;
                        Direction dir = collision.direction;
                        vec2 diff = collision.difference;

                        if (ori == VERTICAL)
                        {
                            vel.y *= -0.6;

                            if (abs(vel.y) < 1)
                            {
                                vel.y = 0;
                            }

                            double reposition = 1 - abs(diff.y);
                            if (dir == UP)
                            {
                                ball_center.y -= reposition;
                            }
                            else
                            {
                                ball_center.y += reposition;
                            }
                        }
                        else
                        {
                            vel.x *= -0.6;

                            double reposition = 1 - abs(diff.x);
                            if (dir == LEFT)
                            {
                                ball_center.x += reposition;
                            }
                            else
                            {
                                ball_center.x -= reposition;
                            }
                        }
                    }
                }

                vec2 ball_top = {ball_center.x, ball_center.y + BALL_RADIUS};
                if (ball_top.x >= 13 && ball_top.x <= 20 && ball_top.y <= 0 && ball_top.y >= -3)
                { // ball is inside hoop
                    ball_in_hoop = true;
                }

                if (ball_in_hoop && ball_top.x >= 13 && ball_top.x <= 20 && ball_top.y < -3)
                {
                    shots_made++;
                    ball_in_hoop = false;
                }

                // Collision shot_made = check_ball_collision(bottom_net, ball_center);
                // if (shot_made.collided)
                // {
                //     Orientation ori = shot_made.orientation;
                //     Direction dir = shot_made.direction;

                //     if (ori == VERTICAL && dir == DOWN)
                //     {
                //         shots_made++;
                //     }
                // }

                ball_center += vel / 20;
                shadow_center.x = ball_center.x;

                // ball hits ground
                if (ball_center.y - 1 < -15)
                {
                    // TODO: reset ball after certain number of seconds
                    ball_shot = false;
                    ball_center = initial_ball_center;
                    shadow_center = initial_shadow_center;
                }
            }

            fancy_draw(PV, V, M, 2, triangle_indices, 4, ball_vp, NULL, NULL, {}, vertex_texCoords, "basketball.png");
        }

        { // net
            fancy_draw(PV, V, M, 2, triangle_indices, 4, net_vp, NULL, NULL, {}, vertex_texCoords, "net.png");
        }

        basic_draw(QUADS, PV, 4, bottom_net, monokai.white);
        // basic_draw(QUADS, PV, 4, front_rim_vp, monokai.white);
        // basic_draw(QUADS, PV, 4, front_net_vp, monokai.white);
        // basic_draw(QUADS, PV, 4, back_rim_vp, monokai.white);
        // basic_draw(QUADS, PV, 4, back_net_vp, monokai.white);

        char shots[8];
        sprintf(shots, "%d", shots_made);

        basic_text(PV, shots, V2(20, 15), monokai.white, 100);

        imgui_slider("background", &bg, 0, 4, 'j', 'k', true);
    }
}

int main()
{
    game();
    return 0;
}
