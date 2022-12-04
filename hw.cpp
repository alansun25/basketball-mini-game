#define _CRT_SECURE_NO_WARNINGS
#include "snail.cpp"
#include "cow.cpp"
#include "_cow_supplement.cpp"
#include "jim.cpp"
// #define EIGEN_LINEAR_SOLVER // will discuss Thursday
#include "_cow_optimization.cpp"

int S = 64;
Texture color_buffer;
void set_pixel(int i, int j, vec3 color, bool shadow)
{
    color_buffer.data[4 * (j * S + i) + 0] = (unsigned char)(255 * CLAMP(color.r, 0, 1));
    color_buffer.data[4 * (j * S + i) + 1] = (unsigned char)(255 * CLAMP(color.g, 0, 1));
    color_buffer.data[4 * (j * S + i) + 2] = (unsigned char)(255 * CLAMP(color.b, 0, 1));
    if (shadow)
    {
        color_buffer.data[4 * (j * S + i) + 3] = 255;
    }
    else
    {
        color_buffer.data[4 * (j * S + i) + 3] = 0;
    }
}

struct RayCastResult
{
    bool hit_at_least_one_triangle;
    vec3 base_color;
    double min_t;
    vec3 n_hit;
    vec3 p_hit;
};

RayCastResult cast_ray(BasicTriangleMesh3D *mesh, vec3 dir, vec3 o)
{
    bool hit_at_least_one_triangle = false;
    vec3 base_color = {};
    double min_t = INFINITY;
    vec3 n_hit = {};

    int num_triangles = mesh->num_vertices / 3;
    for (int triangle_i = 0; triangle_i < num_triangles; ++triangle_i)
    {
        vec3 a, b, c;
        vec3 color_a, color_b, color_c;
        vec3 normal;

        a = mesh->vertex_positions[3 * triangle_i + 0];
        b = mesh->vertex_positions[3 * triangle_i + 1];
        c = mesh->vertex_positions[3 * triangle_i + 2];
        vec3 e1 = b - a;
        vec3 e2 = c - a;
        normal = normalized(cross(e1, e2));

        mat4 A = M4(a.x, b.x, c.x, -dir.x, a.y, b.y, c.y, -dir.y, a.z, b.z, c.z, -dir.z, 1, 1, 1, 0);
        vec4 x = V4(o.x, o.y, o.z, 1);
        vec4 alpha_beta_gamma_t = inverse(A) * x;

        double alpha = alpha_beta_gamma_t.x;
        double beta = alpha_beta_gamma_t.y;
        double gamma = alpha_beta_gamma_t.z;
        double t = alpha_beta_gamma_t.w;

        if (alpha > -TINY && beta > -TINY && gamma > -TINY && t > TINY)
        {
            hit_at_least_one_triangle = true;
            if (t < min_t)
            {
                if (mesh->vertex_colors != NULL)
                {
                    color_a = mesh->vertex_colors[3 * triangle_i + 0];
                    color_b = mesh->vertex_colors[3 * triangle_i + 1];
                    color_c = mesh->vertex_colors[3 * triangle_i + 2];
                }
                else
                {
                    vec3 fallback_color = V3(.5, .5, .5) + .5 * normal;
                    color_a = fallback_color;
                    color_b = fallback_color;
                    color_c = fallback_color;
                }
                base_color = alpha * color_a + beta * color_b + gamma * color_c;
                min_t = t;
                n_hit = normal;
            }
        }
    }

    return {hit_at_least_one_triangle, base_color, min_t, n_hit, o + min_t * dir};
}

double shot_force(vec2 start, vec2 end)
{
    double dist = sqrt((start.x - end.x) * (start.x - end.x) + (start.y - end.y) * (start.y - end.y));
    double shot_force = dist * 15;

    return shot_force;
}

void reset_ball(vec3 ball_vertex_positions[], vec3 ball_shadow_vertex_positions[])
{
    ball_vertex_positions[0] = {-21, -8, 0};
    ball_vertex_positions[1] = {-19, -8, 0};
    ball_vertex_positions[2] = {-19, -6, 0};
    ball_vertex_positions[3] = {-21, -6, 0};

    ball_shadow_vertex_positions[0] = {-21, -17.5, 0};
    ball_shadow_vertex_positions[1] = {-19, -17.5, 0};
    ball_shadow_vertex_positions[2] = {-19, -16.5, 0};
    ball_shadow_vertex_positions[3] = {-21, -16.5, 0};
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

    vec3 ball_vertex_positions[] = {
        {-21, -8, 0},
        {-19, -8, 0},
        {-19, -6, 0},
        {-21, -6, 0},
    };

    vec3 ball_shadow_vertex_positions[] = {
        {-21, -17.5, 0},
        {-19, -17.5, 0},
        {-19, -16.5, 0},
        {-21, -16.5, 0},
    };

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
        {10, -2, 0},
        {12, -2, 0},
        {12, 0, 0},
        {10, 0, 0},
    };

    double time = 0;
    double acc = -9.81 * 0.675;
    double vel_x;
    double vel_y;
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

        // background
        fancy_draw(PV, V, M, 2, triangle_indices, 4, bg_vertex_positions, NULL, NULL, {}, vertex_texCoords, bgs[bg]);

        // blacktop
        basic_draw(QUADS, PV, 4, blacktop, blacktop_colors[bg]);
        basic_draw(LINE_STRIP, PV, 4, blacktop_lines, monokai.white);

        // hoop
        fancy_draw(PV * Scaling({1.5, 1.5, 0}), V, M, 2, triangle_indices, 4, net_vertex_positions, NULL, NULL, {}, vertex_texCoords, "net.png");

        { // connect rim to backboard
            gl_PV(PV);
            gl_begin(QUADS);
            gl_color(V3(253, 177, 89) / 255.);
            gl_vertex(12 * 1.5, -.1 * 1.5);
            gl_vertex(12 * 1.5, 0 * 1.5);
            gl_vertex(12 * 1.5 + 1, 0 * 1.5);
            gl_vertex(12 * 1.5 + 1, -.1 * 1.5);
            gl_end();
        }

        { // backboard
            gl_color(monokai.white);
            gl_begin(QUADS);
            gl_vertex(12 * 1.5 + 1.2, 3.5 * 1.5);
            gl_vertex(12 * 1.5 + 1, 3.5 * 1.5);
            gl_vertex(12 * 1.5 + 1, -1.5 * 1.5);
            gl_vertex(12 * 1.5 + 1.2, -1.5 * 1.5);
            gl_end();
        }

        { // stand
            gl_begin(QUADS);
            gl_vertex(12 * 1.5 + 1.2, 1.5 * 1.5);
            gl_vertex(14.5 * 1.5, 1.5 * 1.5);
            gl_vertex(14.5 * 1.5, 1 * 1.5);
            gl_vertex(12 * 1.5 + 1.2, 1 * 1.5);
            gl_end();

            gl_begin(QUADS);
            gl_vertex(14.5 * 1.5, 1 * 1.5);
            gl_vertex(14 * 1.5, 1 * 1.5);
            gl_vertex(14 * 1.5, -17);
            gl_vertex(14.5 * 1.5, -17);
            gl_end();
        }

        // ball
        static vec2 start_drag;
        static vec2 end_drag;
        if (input.mouse_left_pressed)
        {
            start_drag = input_get_mouse_position_in_world_coordinates(PV);
        }

        if (input.mouse_left_released)
        {
            end_drag = input_get_mouse_position_in_world_coordinates(PV);
            vel_x = -(end_drag.x - start_drag.x) / 10; // TODO: is there a better way to do this?
            vel_y = -(end_drag.y - start_drag.y) * 25; // TODO: is there a better way to do this?
            ball_shot = true;
        }

        if (ball_shot)
        {
            // TODO: Gravity seems weird
            vel_y += acc;
            for (int i = 0; i < 4; i++)
            {
                if (ball_vertex_positions[i].y < -15)
                {
                    ball_shot = false;
                    time = 0;
                    reset_ball(ball_vertex_positions, ball_shadow_vertex_positions);
                    break;
                }

                double original_y = i < 2 ? -8 : -6;
                ball_vertex_positions[i].y = original_y + vel_y * time;
                ball_vertex_positions[i].x += vel_x;

                ball_shadow_vertex_positions[i].x += vel_x;
            }
            time += 3. / 1000;
        }

        fancy_draw(PV, V, M, 2, triangle_indices, 4, ball_vertex_positions, NULL, NULL, {}, vertex_texCoords, "basketball.png");

        // ball shadow
        fancy_draw(PV, V, M, 2, triangle_indices, 4, ball_shadow_vertex_positions, NULL, NULL, {}, vertex_texCoords, "basketball.png", true);

        imgui_slider("background", &bg, 0, 4, 'j', 'k', true);
    }
}

int main()
{
    game();
    return 0;
}
