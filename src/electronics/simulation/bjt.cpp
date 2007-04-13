/***************************************************************************
 *   Copyright (C) 2003-2005 by David Saxton                               *
 *   david@bluehaze.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "bjt.h"
#include "diode.h"
#include "elementset.h"

#include <cmath>
using namespace std;

//BEGIN class BJTSettings
BJTSettings::BJTSettings()
{
	I_S = 1e-16;
	N_F = 1.0;
	N_R = 1.0;
	B_F = 100.0;
	B_R = 1.0;
}
//END class BJTSettings

//BEGIN class BJTState
BJTState::BJTState() {
	reset();
}

void BJTState::reset() {
	for(unsigned i = 0; i < 3; ++i ) {
		for(unsigned j = 0; j < 3; ++j ) A[i][j] = 0.0;

		I[i] = 0.0;
	}
}

BJTState BJTState::operator-(const BJTState & s ) const {
	BJTState newState(*this );

	for(unsigned i = 0; i < 3; ++i ) {
		for(unsigned j = 0; j < 3; ++j ) newState.A[i][j] -= s.A[i][j];

		newState.I[i] -= s.I[i];
	}

	return newState;
}
//END class BJTState


//BEGIN class BJT
BJT::BJT(const bool isNPN )
	: NonLinear()
{
	V_BE_prev = 0.0;
	V_BC_prev = 0.0;
	m_pol = isNPN ? 1 : -1;
	m_numCNodes = 3;
}

BJT::~BJT()
{
}

void BJT::add_map()
{
	if(!b_status) return;

	if(!p_cnode[0]->isGround ) {
		p_A->setUse(p_cnode[0]->n(), p_cnode[0]->n(), Map::et_unstable, false );
	}

	if(!p_cnode[1]->isGround ) {
		p_A->setUse(p_cnode[1]->n(), p_cnode[1]->n(), Map::et_unstable, false );
	}

	if(!p_cnode[2]->isGround ) {
		p_A->setUse(p_cnode[2]->n(), p_cnode[2]->n(), Map::et_unstable, false );
	}
	
	if(!p_cnode[0]->isGround && !p_cnode[2]->isGround ) {
		p_A->setUse(p_cnode[0]->n(), p_cnode[2]->n(), Map::et_unstable, false );
		p_A->setUse(p_cnode[2]->n(), p_cnode[0]->n(), Map::et_unstable, false );
	}

	if(!p_cnode[1]->isGround && !p_cnode[2]->isGround ) {
		p_A->setUse(p_cnode[2]->n(), p_cnode[1]->n(), Map::et_unstable, false );
		p_A->setUse(p_cnode[1]->n(), p_cnode[2]->n(), Map::et_unstable, false );
	}
}

void BJT::add_initial_dc()
{
	V_BE_prev = 0.0;
	V_BC_prev = 0.0;
	m_os.reset();
	update_dc();
}

void BJT::updateCurrents()
{
	if(!b_status) return;

	double V_B = p_cnode[0]->v;
	double V_C = p_cnode[1]->v;
	double V_E = p_cnode[2]->v;

	double V_BE = (V_B - V_E) * m_pol;
	double V_BC = (V_B - V_C) * m_pol;

	double I_BE, I_BC, I_T, g_BE, g_BC, g_IF, g_IR;
	calcIg(V_BE, V_BC, & I_BE, &I_BC, &I_T, &g_BE, &g_BC, &g_IF, &g_IR );

	m_cnodeI[1] = I_BC - I_T;
	m_cnodeI[2] = I_BE + I_T;
	m_cnodeI[0] = -(m_cnodeI[1] + m_cnodeI[2]);
}


void BJT::update_dc()
{
	if(!b_status) return;

	calc_eq();

	BJTState diff = m_ns - m_os;
	for(unsigned i = 0; i < 3; ++i ) {
		for(unsigned j = 0 ; j < 3; ++j ) A_g(i, j ) += diff.A[i][j];

		b_i(i ) += diff.I[i];
	}

	m_os = m_ns;
}


void BJT::calc_eq()
{
	double V_B = p_cnode[0]->v;
	double V_C = p_cnode[1]->v;
	double V_E = p_cnode[2]->v;

	double V_BE = (V_B - V_E) * m_pol;
	double V_BC = (V_B - V_C) * m_pol;

	double I_S = m_bjtSettings.I_S;
	double N_F = m_bjtSettings.N_F;
	double N_R = m_bjtSettings.N_R;

	// adjust voltage to help convergence
	double V_BEcrit = diodeCriticalVoltage(I_S, N_F * V_T );
	double V_BCcrit = diodeCriticalVoltage(I_S, N_R * V_T );
	V_BE_prev = V_BE = diodeVoltage(V_BE, V_BE_prev, V_T * N_F, V_BEcrit );
	V_BC_prev = V_BC = diodeVoltage(V_BC, V_BC_prev, V_T * N_R, V_BCcrit );

	double I_BE, I_BC, I_T, g_BE, g_BC, g_IF, g_IR;
	calcIg(V_BE, V_BC, & I_BE, & I_BC, & I_T, & g_BE, & g_BC, & g_IF, & g_IR );

	double I_eq_B = I_BE - V_BE * g_BE;
	double I_eq_C = I_BC - V_BC * g_BC;
	double I_eq_E = I_T - V_BE * g_IF + V_BC * g_IR;

	m_ns.A[0][0] = g_BC + g_BE;
	m_ns.A[0][1] = -g_BC;
	m_ns.A[0][2] = -g_BE;

	m_ns.A[1][0] = -g_BC + (g_IF - g_IR);
	m_ns.A[1][1] = g_IR + g_BC;
	m_ns.A[1][2] = -g_IF;

	m_ns.A[2][0] = -g_BE - (g_IF - g_IR);
	m_ns.A[2][1] = -g_IR;
	m_ns.A[2][2] = g_BE + g_IF;

	m_ns.I[0] = (-I_eq_B - I_eq_C) * m_pol;
	m_ns.I[1] = (+I_eq_C - I_eq_E) * m_pol;
	m_ns.I[2] = (+I_eq_B + I_eq_E) * m_pol;
}

void BJT::calcIg(double V_BE, double V_BC, double *I_BE, double *I_BC, double *I_T, double *g_BE, double *g_BC, double *g_IF, double *g_IR )
{
	double I_S = m_bjtSettings.I_S;
	double N_F = m_bjtSettings.N_F;
	double N_R = m_bjtSettings.N_R;
	double B_F = m_bjtSettings.B_F;
	double B_R = m_bjtSettings.B_R;

	// base-emitter diodes
	double g_tiny = (V_BE < (-10 * V_T * N_F)) ? I_S : 0;

	double I_F;
	diodeJunction(V_BE, I_S, V_T * N_F, & I_F, g_IF );

	double I_BEI = I_F / B_F;
	double g_BEI = *g_IF / B_F;
	double I_BEN = g_tiny * V_BE;
	double g_BEN = g_tiny;
	*I_BE = I_BEI + I_BEN;
	*g_BE = g_BEI + g_BEN;

	// base-collector diodes
	g_tiny = (V_BC < (-10 * V_T * N_R)) ? I_S : 0;

	double I_R;
	diodeJunction(V_BC, I_S, V_T * N_R, & I_R, g_IR );

	double I_BCI = I_R / B_R;
	double g_BCI = *g_IR / B_R;
	double I_BCN = g_tiny * V_BC;
	double g_BCN = g_tiny;
	*I_BC = I_BCI + I_BCN;
	*g_BC = g_BCI + g_BCN;

	*I_T = I_F - I_R;
}

void BJT::setBJTSettings(const BJTSettings & settings )
{
	m_bjtSettings = settings;

	if(p_eSet) p_eSet->setCacheInvalidated();
}

//END class BJT


