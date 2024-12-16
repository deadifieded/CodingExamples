#pragma once

#include <iostream>
#include <windows.h>
#include <math.h>
#include <random>
#include <list>

double getDistanceSquared(double x1, double y1, double x2, double y2)
{
    return pow(x2 - x1, 2) + pow(y2 - y1, 2);
}

double getDistanceSquared(double dx, double dy)
{
    return pow(dx, 2) + pow(dy, 2);
}

struct SystemState {
    int n_particles;

    double* x_pos;
    double* y_pos;
    double* x_vel;
    double* y_vel;

    unsigned char* particle_types;

    SystemState() {};

    SystemState(int n_particles) : n_particles(n_particles)
    {
        x_pos = new double[n_particles];
        y_pos = new double[n_particles];
        x_vel = new double[n_particles];
        y_vel = new double[n_particles];
        particle_types = new unsigned char[n_particles];
    }

    ~SystemState()
    {
        delete[] x_pos;
        delete[] y_pos;
        delete[] x_vel;
        delete[] y_vel;
    }

    SystemState *generateCompatibleState()
    {
        return new SystemState(n_particles);
    }

    void randomize(
        double x_min, double x_max, double y_min, double y_max, 
        double x_v_min, double x_v_max, double y_v_min, double y_v_max, 
        std::default_random_engine generator, std::uniform_real_distribution<double> distribution)
    {
        for (int i = 0; i < n_particles; i += 1) 
        {
            x_pos[i] = distribution(generator) * (x_max - x_min) + x_min;
            y_pos[i] = distribution(generator) * (y_max - y_min) + y_min;
            x_vel[i] = distribution(generator) * (x_v_max - x_v_min) + x_v_min;
            y_vel[i] = distribution(generator) * (y_v_max - y_v_min) + y_v_min;
        }
    }
};

class ParticleBoxThing {

private:

    const int velocity_div = 2 * 2 * 2 * 2 * 2 * 2 * 2 * 2;

    const int state_rule[3][3] = { {0,0,0},{1,1,2},{0,2,2} };

    COLORREF type_colors[4] = { RGB(255,0,0), RGB(0,0,0), RGB(0,0,255), RGB(0,0,0)};

    const double ts = 0.001;


    //graphics stuff

    HDC* cell_hdcs;

    HBITMAP cell_mask;

    HDC cell_mask_hdc;

    int cell_size;

    double scaling;

    bool first = true;


    //initial conditions

    int n_particles;

    double particle_rad;
    double double_particle_radius_squared;

    double ex_rad;
    double ex_collision_rad_squared;
    double ex_pos_x;
    double ex_pos_y;

    double ionis_rad;
    double ionis_collision_rad_squared;
    double ionis_pos_x;
    double ionis_pos_y;
    double ions_decay_rate;

    double x_min, x_max;
    double y_min, y_max;
    double x_v_min, x_v_max;
    double y_v_min, y_v_max;

    double x_length;
    double y_length;

    //particle info

    SystemState *states[2];
    int recent_state;


    //chunk stuff

    double chunk_size;

    int buffer_size;

    int nof_chunk_xs;
    int nof_chunk_ys;

    std::list<int>** chunks;


    std::default_random_engine generator;

    std::uniform_real_distribution<double> distribution;

    int getChunkCoord(double x)
    {
        return floor(x / chunk_size) + buffer_size;
    }

    void setupChunks(SystemState* state)
    {
        chunks = new std::list<int>*[nof_chunk_xs];

        for (int i = 0; i < nof_chunk_xs; i += 1)
        {
            chunks[i] = new std::list<int>[nof_chunk_ys];

            for (int j = 0; j < nof_chunk_ys; j += 1)
            {
                chunks[i][j] = std::list<int>();
            }
        }

        int chunk_x;
        int chunk_y;

        for (int i = 0; i < n_particles; i += 1)
        {
            chunk_x = getChunkCoord(state->x_pos[i]);
            chunk_y = getChunkCoord(state->y_pos[i]);

            chunks[chunk_x][chunk_y].push_back(i);
        }
    }

    void deleteChunks()
    {
        for (int i = 0; i < nof_chunk_xs;i += 1)
        {
            delete[] chunks[i];
        }

        delete[] chunks;
    }

    void setParticleTypes(SystemState *state) {
        double x;
        double y;
        for (int i = 0; i < n_particles; i += 1)
        {
            x = state->x_pos[i];
            y = state->y_pos[i];

            if (getDistanceSquared(x, y, ex_pos_x, ex_pos_y) < ex_collision_rad_squared)
            {
                state->particle_types[i] = 2;
            }
            else if (getDistanceSquared(x, y, ionis_pos_x, ionis_pos_y) < ionis_collision_rad_squared)
            {
                state->particle_types[i] = 0;
            }
            else
            {
                state->particle_types[i] = 1;
            }
        }
    }

    void setDefaultValues() {
        generator = std::default_random_engine();
        distribution = std::uniform_real_distribution<double>(0, 1);

        cell_hdcs = nullptr;

        //initial conditions
        particle_rad = 0.01;
        double_particle_radius_squared = 4 * particle_rad * particle_rad;

        ex_rad = 0.01;
        ex_collision_rad_squared = (ex_rad + particle_rad) * (ex_rad + particle_rad);
        ex_pos_x = 0.25;
        ex_pos_y = 0.5;

        ionis_rad = 0.01;
        ionis_collision_rad_squared = (ionis_rad + particle_rad) * (ionis_rad + particle_rad);
        ionis_pos_x = 0.75;
        ionis_pos_y = 0.5;
        ions_decay_rate = 0.5;

        x_min = 0;
        x_max = 1;
        y_min = 0;
        y_max = 1;
        x_v_min = -1 * ts;
        x_v_max = 1 * ts;
        y_v_min = -1 * ts;
        y_v_max = 1 * ts;
    }

    void discardGraphicsResources()
    {
        if (cell_hdcs != nullptr)
        {
            for (int i = 0; i < 3; i += 1) {
                DeleteDC(cell_hdcs[i]);
            }

        }
    }

public:

    ParticleBoxThing(int n_particles) : n_particles(n_particles)
    {
        setDefaultValues();

        x_length = x_max - x_min;
        y_length = y_max - y_min;
        
        states[0] = new SystemState(n_particles);
        states[1] = states[0]->generateCompatibleState();

        states[0]->randomize(x_min, x_max, y_min, y_max, x_v_min, x_v_max, y_v_min, y_v_max, generator, distribution);


        chunk_size = 2.1 * particle_rad;

        buffer_size = 1;

        nof_chunk_xs = floor((x_max - x_min) / chunk_size) + 1 + 2 * buffer_size;
        nof_chunk_ys = floor((y_max - y_min) / chunk_size) + 1 + 2 * buffer_size;

        
        setupChunks(states[0]);

        setParticleTypes(states[0]);

        recent_state = 0;    
    }

    ~ParticleBoxThing()
    {
        delete states[0];
        delete states[1];
        deleteChunks();
        discardGraphicsResources();
    }

    void update()
    {
        /*int chunk1_x, chunk1_y;

        int chunk_start_x, chunk_cap_x, chunk_start_y, chunk_cap_y;

        std::list<int>* chunk_column;

        std::list<int>* chunk1, * chunk2;

        double x1, x2, y1, y2;
        double v_x1, v_x2, v_y1, v_y2;

        double* cur_x_pos, * cur_y_pos, * new_x_pos, * new_y_pos;
        double* cur_x_vel, * cur_y_vel, * new_x_vel, * new_y_vel;

        int type1, type2;
        unsigned char* cur_types, * new_types;

        double dist;*/
        int new_state = (recent_state + 1) % 2;

        double* cur_x_pos = states[recent_state]->x_pos;
        double* cur_y_pos = states[recent_state]->y_pos;
        double* cur_x_vel = states[recent_state]->x_vel;
        double* cur_y_vel = states[recent_state]->y_vel;

        double* new_x_pos = states[new_state]->x_pos;
        double* new_y_pos = states[new_state]->y_pos;
        double* new_x_vel = states[new_state]->x_vel;
        double* new_y_vel = states[new_state]->y_vel;

        unsigned char* cur_types = states[recent_state]->particle_types;
        unsigned char* new_types = states[new_state]->particle_types;

        for (int p1 = 0; p1 < n_particles; p1 += 1)
        {
            new_types[p1] = cur_types[p1];
            if (new_types[p1] == 0)
            {
                if (distribution(generator) < ts * ions_decay_rate)
                {
                    new_types[p1] = 1;
                }
            }
            new_x_vel[p1] = cur_x_vel[p1];
            new_y_vel[p1] = cur_y_vel[p1];
        }

        for (int p1 = 0; p1 < n_particles; p1 += 1)
        {
            double x1 = cur_x_pos[p1];
            double y1 = cur_y_pos[p1];

            double v_x1 = new_x_vel[p1];
            double v_y1 = new_y_vel[p1];

            unsigned char type1 = new_types[p1];

            

            int chunk1_x = getChunkCoord(x1);
            int chunk1_y = getChunkCoord(y1);

            std::list<int>* chunk1 = &chunks[chunk1_x][chunk1_y];

            (*chunk1).pop_front();

            int chunk_start_x = chunk1_x - 1;
            int chunk_cap_x = chunk1_x + 2;
            int chunk_start_y = chunk1_y - 1;
            int chunk_cap_y = chunk1_y + 2;

            if (chunk_start_x < 0)
            {
                chunk_start_x = 0;
            }
            else if (chunk_cap_x > nof_chunk_xs)
            {
                chunk_cap_x = nof_chunk_xs;
            }

            if (chunk_start_y < 0)
            {
                chunk_start_y = 0;
            }
            else if (chunk_cap_y > nof_chunk_ys)
            {
                chunk_cap_y = nof_chunk_ys;
            }



            for (int chunk2_x = chunk_start_x; chunk2_x < chunk_cap_x; chunk2_x += 1)
            {
                std::list<int>* chunk_column = chunks[chunk2_x];

                for (int chunk2_y = chunk_start_y; chunk2_y < chunk_cap_y; chunk2_y += 1)
                {
                    std::list<int>* chunk2 = &chunk_column[chunk2_y];

                    for (int p2 : (*chunk2))
                    {
                        if (p2 <= p1) {
                            continue;
                        }

                        double x2 = cur_x_pos[p2];
                        double y2 = cur_y_pos[p2];

                        double v_x2 = new_x_vel[p2];
                        double v_y2 = new_y_vel[p2];

                        unsigned char type2 = new_types[p2];

                        double dx = x2 - x1;
                        double dy = y2 - y1;

                        double dist_squared = getDistanceSquared(dx, dy);

                        if (dist_squared < double_particle_radius_squared)
                        {
                            if (getDistanceSquared(x1 + v_x1 / velocity_div, y1 + v_y1 / velocity_div, x2 + v_x2 / velocity_div, y2 + v_y2 / velocity_div) < dist_squared)
                            {
                                double dist = sqrt(dist_squared);


                                //unit displacement from p1 to p2
                                double q_x = dx / dist;
                                double q_y = dy / dist;

                                double a = (v_x2 - v_x1) * q_x + (v_y2 - v_y1) * q_y;

                                double dvx = a*q_x;
                                double dvy = a*q_y;

                                v_x1 += dvx;
                                v_y1 += dvy;
                                new_x_vel[p2] -= dvx;
                                new_y_vel[p2] -= dvy;
                                new_x_vel[p1] = v_x1;
                                new_y_vel[p1] = v_y1;

                            }

                            new_types[p1] = state_rule[type1][type2];
                            new_types[p2] = state_rule[type2][type1];
                            type1 = new_types[p1];
                        }
                    }
                }
            }



            if (x1 > x_max - particle_rad)
            {
                if (new_x_vel[p1] > 0)
                {
                    new_x_vel[p1] *= -1;
                }
            }
            else if (x1 < x_min + particle_rad)
            {
                if (new_x_vel[p1] < 0)
                {
                    new_x_vel[p1] *= -1;
                }
            }
            else if (getDistanceSquared(x1, y1, ex_pos_x, ex_pos_y) < ex_collision_rad_squared)
            {
                new_types[p1] = 2;
            }
            else if (getDistanceSquared(x1, y1, ionis_pos_x, ionis_pos_y) < ionis_collision_rad_squared)
            {
                new_types[p1] = 0;
            }

            if (y1 > y_max - particle_rad)
            {
                if (new_y_vel[p1] > 0)
                {
                    new_y_vel[p1] *= -1;
                }
            }
            else if (y1 < y_min + particle_rad)
            {
                if (new_y_vel[p1] < 0)
                {
                    new_y_vel[p1] *= -1;
                }
            }

            x1 += new_x_vel[p1];
            y1 += new_y_vel[p1];

            new_x_pos[p1] = x1;
            new_y_pos[p1] = y1;

            chunks[getChunkCoord(x1)][getChunkCoord(y1)].push_back(p1);
        }

        recent_state = new_state;

    }

    void paint(HDC hdc)
    {
        double cell_rad = (double)cell_size / 2;

        double* x_pos = states[recent_state]->x_pos;
        double* y_pos = states[recent_state]->y_pos;

        unsigned char* types = states[recent_state]->particle_types;

       // BitBlt(hdc, 1000,400, cell_size, cell_size, cell_mask_hdc, 0, 0, SRCINVERT);

        for (int i = 0; i < n_particles; i += 1)
        {
            //if (true)
                MaskBlt(hdc, x_pos[i] * scaling - cell_rad, y_pos[i] * scaling - cell_rad, cell_size, cell_size, cell_hdcs[types[i]], 0, 0, cell_mask, 0, 0, MAKEROP4(SRCCOPY, 0x00AA0029));
            //else
                //BitBlt(hdc, x_pos[i] * scaling - cell_rad, y_pos[i] * scaling - cell_rad, cell_size, cell_size, cell_hdcs[types[i]], 0, 0, SRCCOPY);
        }
    }

    void createGraphicsResources(int width, int height, HDC hdc)
    {
        scaling = width / x_length;

        if (height / y_length < scaling)
            scaling = height / y_length;

        cell_size = scaling * particle_rad * 2;

        if (cell_hdcs != nullptr)
        {
            for (int i = 0; i < 3; i += 1) 
            {
                DeleteDC(cell_hdcs[i]);
            }
        }

        double cell_rad = (double)(cell_size) / 2;

        cell_hdcs = new HDC[3];



        for (int i = 0; i < 3; i += 1) {

            HBITMAP h_bitmap = CreateCompatibleBitmap(hdc, cell_size, cell_size);
            cell_hdcs[i] = CreateCompatibleDC(hdc);
            COLORREF color = type_colors[i];//m_progress((float)i/num_divs);
            SelectObject(cell_hdcs[i], h_bitmap);

            for (int j = 0; j < cell_size;j += 1)
            {
                for (int k = 0;k < cell_size; k += 1)
                {
                    SetPixel(cell_hdcs[i], j, k, color);
                }
            }
        }

        cell_mask_hdc = CreateCompatibleDC(hdc);



        unsigned char* bits = new unsigned char[cell_size * cell_size];

        int n = 0;
        int pos = 0;

        for (int x = 0; x < cell_size; x += 1)
        {
            bits[n] = 0;

            if (n % 2 == 1)
            {
                n += 1;
                bits[n] = 0;
            }
                

            for (int y = 0; y < cell_size; y += 1)
            {
                double dx = (double)x - cell_rad + 0.5;
                double dy = (double)y - cell_rad + 0.5;

                if (dx * dx + dy * dy > (double)cell_rad * (double)cell_rad)
                {
                    bits[n] += 0;
                }
                else
                {
                    bits[n] += 1;
                }

                pos += 1;

                if (pos < 8)
                {
                    bits[n] *= 2;
                }
                else
                {
                    n += 1;
                    pos = 0;
                    bits[n] = 0;
                }
            }

            if (pos != 0)
            {
                pos += 1;
                while (pos < 8)
                {
                    pos += 1;
                    bits[n] *= 2;
                }
                n += 1;
                pos = 0;
            }
        }



        cell_mask = CreateBitmap(cell_size, cell_size, 1, 1, bits);
        SelectObject(cell_mask_hdc, cell_mask);
    }
};
