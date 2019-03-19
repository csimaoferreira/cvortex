#include "libcvtx.h"
/*============================================================================
StraightVortFil.c

Basic representation of a straight vortex filament.

Copyright(c) 2019 HJA Bird

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
============================================================================*/
#include <assert.h>
#include <math.h>

static const float pi_f = (float) 3.14159265359;

CVTX_EXPORT bsv_V3f cvtx_StraightVortFil_ind_vel(
	const cvtx_StraightVortFil *self,
	const bsv_V3f mes_point) 
{

	assert(self != NULL);
	bsv_V3f r0, r1, r2, crosstmp;
	float t1, t2, t21, t22;
	r1 = bsv_V3f_minus(mes_point, self->start);
	r2 = bsv_V3f_minus(mes_point, self->end);
	r0 = bsv_V3f_minus(r1, r2);
	crosstmp = bsv_V3f_cross(r1, r2);
	t1 = self->strength / (4 * pi_f * powf(bsv_V3f_abs(crosstmp), 2));
	t21 = bsv_V3f_dot(r1, r0) / bsv_V3f_abs(r1);
	t22 = bsv_V3f_dot(r2, r0) / bsv_V3f_abs(r2);
	t2 = t21 - t22;
	return bsv_V3f_mult(crosstmp, t1 * t2);
}

CVTX_EXPORT bsv_V3f cvtx_StraightVortFil_ind_dvort(
	const cvtx_StraightVortFil *self,
	const cvtx_Particle *induced_particle) 
{
	assert(self != NULL);
	/* HJAB, Notes 4, pg.42 - pg. 43 for general theme. */
	bsv_V3f r0, r1, r2, t211, A, ret, tmp;
	float t1, t2121, t2122, t221, t2221, t2222, B;
	r1 = bsv_V3f_minus(induced_particle->coord, self->start);
	r2 = bsv_V3f_minus(induced_particle->coord, self->end);
	r0 = bsv_V3f_minus(r1, r2);
	t1 = self->strength / (4 * pi_f);
	t211 = bsv_V3f_div(r0, -powf(bsv_V3f_abs(bsv_V3f_cross(r1, r0)), 2));
	t2121 = bsv_V3f_dot(r0, r1) / bsv_V3f_abs(r1);
	t2122 = -bsv_V3f_dot(r0, r2) / bsv_V3f_abs(r2);
	t221 = (float)3.0 / bsv_V3f_abs(r0);
	t2221 = bsv_V3f_abs(bsv_V3f_cross(r0, r1)) / bsv_V3f_abs(r1);
	t2222 = -bsv_V3f_abs(bsv_V3f_cross(r0, r1)) / bsv_V3f_abs(r2);
	A = bsv_V3f_mult(t211, t1*(t2121 + t2122));
	B = t221 * t1 * (t2221 + t2222);
	tmp.x[0] = B;	tmp.x[1] = -A.x[2];	tmp.x[2] = A.x[1];
	ret.x[0] = bsv_V3f_dot(tmp, induced_particle->vorticity);
	tmp.x[0] = A.x[2];	tmp.x[1] = B;	tmp.x[2] = -A.x[0];
	ret.x[1] = bsv_V3f_dot(tmp, induced_particle->vorticity);
	tmp.x[0] = -A.x[1];	tmp.x[1] = A.x[0];	tmp.x[2] = B;
	ret.x[2] = bsv_V3f_dot(tmp, induced_particle->vorticity);
	return ret;
};

CVTX_EXPORT bsv_V3f cvtx_StraightVortFilArr_ind_vel(
	const cvtx_StraightVortFil **array_start,
	const long num_particles,
	const bsv_V3f mes_point) 
{
	assert(num_particles >= 0);
	assert(array_start != NULL);
	bsv_V3f vel;
	double rx = 0, ry = 0, rz = 0;
	long i;
	for (i = 0; i < num_particles; ++i) {
		vel = cvtx_StraightVortFil_ind_vel(array_start[i],
			mes_point);
		rx += vel.x[0];
		ry += vel.x[1];
		rz += vel.x[2];
	}
	bsv_V3f ret = { (float)rx, (float)ry, (float)rz };
	return ret;
}

CVTX_EXPORT bsv_V3f cvtx_StraightVortFilArr_ind_dvort(
	const cvtx_StraightVortFil **array_start,
	const long num_particles,
	const cvtx_Particle *induced_particle) 
{
	assert(num_particles >= 0);
	assert(array_start != NULL);
	bsv_V3f dvort;
	double rx = 0, ry = 0, rz = 0;
	long i;
	for (i = 0; i < num_particles; ++i) {
		dvort = cvtx_StraightVortFil_ind_dvort(array_start[i],
			induced_particle);
		rx += dvort.x[0];
		ry += dvort.x[1];
		rz += dvort.x[2];
	}
	bsv_V3f ret = { (float)rx, (float)ry, (float)rz };
	return ret;
}

CVTX_EXPORT void cvtx_StraightVortFilArr_Arr_ind_vel(
	const cvtx_StraightVortFil **array_start,
	const long num_particles,
	const bsv_V3f *mes_start,
	const long num_mes,
	bsv_V3f *result_array) 
{
	long i;
#pragma omp parallel for schedule(static)
	for (i = 0; i < num_mes; ++i) {
		result_array[i] = cvtx_StraightVortFilArr_ind_vel(
			array_start, num_particles, mes_start[i]);
	}
	return;
}

CVTX_EXPORT void cvtx_StraightVortFilArr_Arr_ind_dvort(
	const cvtx_StraightVortFil **array_start,
	const long num_particles,
	const cvtx_Particle **induced_start,
	const long num_induced,
	bsv_V3f *result_array) 
{
	long i;
#pragma omp parallel for schedule(static)
	for (i = 0; i < num_induced; ++i) {
		result_array[i] = cvtx_StraightVortFilArr_ind_dvort(
			array_start, num_particles, induced_start[i]);
	}
	return;
}