
/* GMRFLib-sparse-interface.c
 * 
 * Copyright (C) 2001-2006 Havard Rue
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * The author's contact information:
 *
 *       H{\aa}vard Rue
 *       Department of Mathematical Sciences
 *       The Norwegian University of Science and Technology
 *       N-7491 Trondheim, Norway
 *       Voice: +47-7359-3533    URL  : http://www.math.ntnu.no/~hrue  
 *       Fax  : +47-7359-3524    Email: havard.rue@math.ntnu.no
 *
 */

/*!
  \file sparse-interface.c
  \brief Unified interface to the sparse-matrix libraries
*/

#include "GMRFLib/GMRFLib.h"
#include "GMRFLib/GMRFLibP.h"

#ifndef HGVERSION
#define HGVERSION
#endif
static const char RCSId[] = "file: " __FILE__ "  " HGVERSION;

/* Pre-hg-Id: $Id: sparse-interface.c,v 1.41 2010/02/27 08:32:02 hrue Exp $ */

/*!
  \brief Compute the reordering
*/
int GMRFLib_compute_reordering(GMRFLib_sm_fact_tp * sm_fact, GMRFLib_graph_tp * graph)
{
	GMRFLib_ENTER_ROUTINE;

	switch (GMRFLib_reorder) {
	case GMRFLib_REORDER_DEFAULT:
		/*
		 * this choice depends on the sparse-solver 
		 */
		switch (sm_fact->smtp) {
		case GMRFLib_SMTP_BAND:
			GMRFLib_EWRAP1(GMRFLib_compute_reordering_BAND(&(sm_fact->remap), graph));
			break;
		case GMRFLib_SMTP_PROFILE:
			GMRFLib_EWRAP1(GMRFLib_compute_reordering_PROFILE());
			break;
		case GMRFLib_SMTP_TAUCS:
			GMRFLib_EWRAP1(GMRFLib_compute_reordering_TAUCS(&(sm_fact->remap), graph, GMRFLib_reorder));
			break;
		default:
			GMRFLib_ASSERT(1 == 0, GMRFLib_ESNH);
			break;
		}
		break;

	case GMRFLib_REORDER_BAND:
		GMRFLib_EWRAP1(GMRFLib_compute_reordering_BAND(&(sm_fact->remap), graph));
		break;

		/*
		 * all the remaining ones are treated by the _TAUCS routine 
		 */
	case GMRFLib_REORDER_IDENTITY:
	case GMRFLib_REORDER_METIS:
	case GMRFLib_REORDER_GENMMD:
	case GMRFLib_REORDER_AMD:
	case GMRFLib_REORDER_MD:
	case GMRFLib_REORDER_MMD:
		GMRFLib_EWRAP1(GMRFLib_compute_reordering_TAUCS(&(sm_fact->remap), graph, GMRFLib_reorder));
		break;
	default:
		GMRFLib_ASSERT(0 == 1, GMRFLib_ESNH);
	}

	if (sm_fact->remap) {				       /* need this still for the wa-routines. FIXME */
		GMRFLib_EWRAP1(GMRFLib_compute_bandwidth(&(sm_fact->bandwidth), graph, sm_fact->remap));
	} else {
		sm_fact->bandwidth = -1;
	}

	GMRFLib_LEAVE_ROUTINE;

	return GMRFLib_SUCCESS;
}

/*!
  \brief Free the reordering
*/
int GMRFLib_free_reordering(GMRFLib_sm_fact_tp * sm_fact)
{
	GMRFLib_ENTER_ROUTINE;

	if (sm_fact) {
		Free(sm_fact->remap);
		sm_fact->bandwidth = 0;
	}
	GMRFLib_LEAVE_ROUTINE;
	return GMRFLib_SUCCESS;
}

/*
  \brief Build a sparse matrix
*/
int GMRFLib_build_sparse_matrix(GMRFLib_sm_fact_tp * sm_fact, GMRFLib_Qfunc_tp * Qfunc, void *Qfunc_arg, GMRFLib_graph_tp * graph)
{
	GMRFLib_ENTER_ROUTINE;
	int ret;

	switch (sm_fact->smtp) {
	case GMRFLib_SMTP_BAND:
		if (GMRFLib_catch_error_for_inla) {
			ret = GMRFLib_build_sparse_matrix_BAND(&(sm_fact->bchol), Qfunc, Qfunc_arg, graph, sm_fact->remap, sm_fact->bandwidth);
			if (ret != GMRFLib_SUCCESS) {
				return ret;
			}
		} else {
			GMRFLib_EWRAP1(GMRFLib_build_sparse_matrix_BAND(&(sm_fact->bchol), Qfunc, Qfunc_arg, graph, sm_fact->remap, sm_fact->bandwidth));
		}
		break;
	case GMRFLib_SMTP_PROFILE:
		GMRFLib_EWRAP1(GMRFLib_build_sparse_matrix_PROFILE());
		break;
	case GMRFLib_SMTP_TAUCS:
		if (GMRFLib_catch_error_for_inla) {
			ret = GMRFLib_build_sparse_matrix_TAUCS(&(sm_fact->L), Qfunc, Qfunc_arg, graph, sm_fact->remap);
			if (ret != GMRFLib_SUCCESS) {
				return ret;
			}
		} else {
			GMRFLib_EWRAP1(GMRFLib_build_sparse_matrix_TAUCS(&(sm_fact->L), Qfunc, Qfunc_arg, graph, sm_fact->remap));
		}
		break;
	default:
		GMRFLib_ASSERT(1 == 0, GMRFLib_ESNH);
		break;
	}

	GMRFLib_LEAVE_ROUTINE;
	return GMRFLib_SUCCESS;
}

/*!
  \brief Factorise a sparse matrix
*/
int GMRFLib_factorise_sparse_matrix(GMRFLib_sm_fact_tp * sm_fact, GMRFLib_graph_tp * graph)
{
	int ret;
	GMRFLib_ENTER_ROUTINE;

	switch (sm_fact->smtp) {
	case GMRFLib_SMTP_BAND:
		if (GMRFLib_catch_error_for_inla) {
			ret = GMRFLib_factorise_sparse_matrix_BAND(sm_fact->bchol, &(sm_fact->finfo), graph, sm_fact->bandwidth);
			if (ret != GMRFLib_SUCCESS) {
				return ret;
			}
		} else {
			GMRFLib_EWRAP1(GMRFLib_factorise_sparse_matrix_BAND(sm_fact->bchol, &(sm_fact->finfo), graph, sm_fact->bandwidth));
		}
		break;
	case GMRFLib_SMTP_PROFILE:
		GMRFLib_EWRAP1(GMRFLib_factorise_sparse_matrix_PROFILE());
		break;
	case GMRFLib_SMTP_TAUCS:
		if (GMRFLib_catch_error_for_inla) {
			ret = GMRFLib_factorise_sparse_matrix_TAUCS(&(sm_fact->L), &(sm_fact->symb_fact), &(sm_fact->finfo), &(sm_fact->L_inv_diag));
			if (ret != GMRFLib_SUCCESS) {
				return ret;
			}
		} else {
			GMRFLib_EWRAP1(GMRFLib_factorise_sparse_matrix_TAUCS(&(sm_fact->L), &(sm_fact->symb_fact), &(sm_fact->finfo), &(sm_fact->L_inv_diag)));
		}
		break;
	default:
		GMRFLib_ASSERT(1 == 0, GMRFLib_ESNH);
		break;
	}

	GMRFLib_LEAVE_ROUTINE;

	return GMRFLib_SUCCESS;
}

/*!
  \brief Free a factorisation of a sparse matrix
*/
int GMRFLib_free_fact_sparse_matrix(GMRFLib_sm_fact_tp * sm_fact)
{
	GMRFLib_ENTER_ROUTINE;

	if (sm_fact) {
		switch (sm_fact->smtp) {
		case GMRFLib_SMTP_BAND:
			GMRFLib_EWRAP1(GMRFLib_free_fact_sparse_matrix_BAND(sm_fact->bchol));
			sm_fact->bchol = NULL;
			break;
		case GMRFLib_SMTP_PROFILE:
			GMRFLib_EWRAP1(GMRFLib_free_fact_sparse_matrix_PROFILE());
			break;
		case GMRFLib_SMTP_TAUCS:
			GMRFLib_EWRAP1(GMRFLib_free_fact_sparse_matrix_TAUCS(sm_fact->L, sm_fact->L_inv_diag, sm_fact->symb_fact));
			sm_fact->L = NULL;
			sm_fact->symb_fact = NULL;
			break;
		default:
			GMRFLib_ASSERT(1 == 0, GMRFLib_ESNH);
			break;
		}
	}
	GMRFLib_LEAVE_ROUTINE;
	return GMRFLib_SUCCESS;
}

/*!
  \brief Solve \f$Lx=b\f$
*/
int GMRFLib_solve_l_sparse_matrix(double *rhs, GMRFLib_sm_fact_tp * sm_fact, GMRFLib_graph_tp * graph)
{
	/*
	 * rhs in real world. solve L x=rhs, rhs is overwritten by the solution 
	 */
	GMRFLib_ENTER_ROUTINE;

	switch (sm_fact->smtp) {
	case GMRFLib_SMTP_BAND:
		GMRFLib_EWRAP1(GMRFLib_solve_l_sparse_matrix_BAND(rhs, sm_fact->bchol, graph, sm_fact->remap, sm_fact->bandwidth));
		break;
	case GMRFLib_SMTP_PROFILE:
		GMRFLib_EWRAP1(GMRFLib_solve_l_sparse_matrix_PROFILE());
		break;
	case GMRFLib_SMTP_TAUCS:
		GMRFLib_EWRAP1(GMRFLib_solve_l_sparse_matrix_TAUCS(rhs, sm_fact->L, graph, sm_fact->remap));
		break;
	default:
		GMRFLib_ERROR(GMRFLib_ESNH);
		break;
	}

	GMRFLib_LEAVE_ROUTINE;

	return GMRFLib_SUCCESS;
}

/*!
  \brief Solve \f$L^Tx=b\f$
*/
int GMRFLib_solve_lt_sparse_matrix(double *rhs, GMRFLib_sm_fact_tp * sm_fact, GMRFLib_graph_tp * graph)
{
	/*
	 * rhs in real world. solve L^Tx=rhs, rhs is overwritten by the solution 
	 */
	GMRFLib_ENTER_ROUTINE;

	switch (sm_fact->smtp) {
	case GMRFLib_SMTP_BAND:
		GMRFLib_EWRAP1(GMRFLib_solve_lt_sparse_matrix_BAND(rhs, sm_fact->bchol, graph, sm_fact->remap, sm_fact->bandwidth));
		break;
	case GMRFLib_SMTP_PROFILE:
		GMRFLib_EWRAP1(GMRFLib_solve_lt_sparse_matrix_PROFILE());
		break;
	case GMRFLib_SMTP_TAUCS:
		GMRFLib_EWRAP1(GMRFLib_solve_lt_sparse_matrix_TAUCS(rhs, sm_fact->L, graph, sm_fact->remap));
		break;
	default:
		GMRFLib_ERROR(GMRFLib_ESNH);
		break;
	}

	GMRFLib_LEAVE_ROUTINE;

	return GMRFLib_SUCCESS;
}

/*!
  \brief Solve \f$LL^Tx=b\f$  or \f$Qx=b\f$
*/
int GMRFLib_solve_llt_sparse_matrix(double *rhs, GMRFLib_sm_fact_tp * sm_fact, GMRFLib_graph_tp * graph)
{
	/*
	 * rhs in real world. solve Q x=rhs, where Q=L L^T 
	 */
	GMRFLib_ENTER_ROUTINE;

	switch (sm_fact->smtp) {
	case GMRFLib_SMTP_BAND:
		GMRFLib_EWRAP1(GMRFLib_solve_llt_sparse_matrix_BAND(rhs, sm_fact->bchol, graph, sm_fact->remap, sm_fact->bandwidth));
		break;
	case GMRFLib_SMTP_PROFILE:
		GMRFLib_EWRAP1(GMRFLib_solve_llt_sparse_matrix_PROFILE());
		break;
	case GMRFLib_SMTP_TAUCS:
		GMRFLib_EWRAP1(GMRFLib_solve_llt_sparse_matrix_TAUCS(rhs, sm_fact->L, graph, sm_fact->remap));
		break;
	default:
		GMRFLib_ERROR(GMRFLib_ESNH);
		break;
	}

	GMRFLib_LEAVE_ROUTINE;

	return GMRFLib_SUCCESS;
}
int GMRFLib_solve_llt_sparse_matrix_special(double *rhs, GMRFLib_sm_fact_tp * sm_fact, GMRFLib_graph_tp * graph, int idx)
{
	/*
	 * rhs in real world. solve Q x=rhs, where Q=L L^T. BUT, here we know that rhs is 0 execpt for a 1 at index idx.
	 */
	GMRFLib_ENTER_ROUTINE;

	switch (sm_fact->smtp) {
	case GMRFLib_SMTP_BAND:
		/*
		 * not implemented yet, so we're using the default (and fail-safe) version 
		 */
		GMRFLib_EWRAP1(GMRFLib_solve_llt_sparse_matrix_BAND(rhs, sm_fact->bchol, graph, sm_fact->remap, sm_fact->bandwidth));
		break;
	case GMRFLib_SMTP_PROFILE:
		/*
		 * not implemented yet, so we're using the default (and fail-safe) version 
		 */
		GMRFLib_EWRAP1(GMRFLib_solve_llt_sparse_matrix_PROFILE());
		break;
	case GMRFLib_SMTP_TAUCS:
		/*
		 * use this special version 
		 */
		GMRFLib_EWRAP1(GMRFLib_solve_llt_sparse_matrix_special_TAUCS(rhs, sm_fact->L, sm_fact->L_inv_diag, graph, sm_fact->remap, idx));
		break;
	default:
		GMRFLib_ERROR(GMRFLib_ESNH);
		break;
	}

	GMRFLib_LEAVE_ROUTINE;

	return GMRFLib_SUCCESS;
}

/*!
  \brief Solve \f$L^Tx=b\f$ for indices in an interval
*/
int GMRFLib_solve_lt_sparse_matrix_special(double *rhs, GMRFLib_sm_fact_tp * sm_fact, GMRFLib_graph_tp * graph, int findx, int toindx, int remapped)
{
	/*
	 * rhs in real world, bchol in mapped world. solve L^Tx=b backward only from rhs[findx] up to rhs[toindx]. note that
	 * findx and toindx is in mapped world. if remapped, do not remap/remap-back the rhs before solving.
	 * 
	 * this routine is called to many times and the work is not that much, to justify GMRFLib_ENTER_ROUTINE; 
	 */
	switch (sm_fact->smtp) {
	case GMRFLib_SMTP_BAND:
		GMRFLib_EWRAP0(GMRFLib_solve_lt_sparse_matrix_special_BAND
			       (rhs, sm_fact->bchol, graph, sm_fact->remap, sm_fact->bandwidth, findx, toindx, remapped));
		break;
	case GMRFLib_SMTP_PROFILE:
		GMRFLib_EWRAP0(GMRFLib_solve_lt_sparse_matrix_special_PROFILE());
		break;
	case GMRFLib_SMTP_TAUCS:
		GMRFLib_EWRAP0(GMRFLib_solve_lt_sparse_matrix_special_TAUCS(rhs, sm_fact->L, graph, sm_fact->remap, findx, toindx, remapped));
		break;
	default:
		GMRFLib_ERROR(GMRFLib_ESNH);
		break;
	}

	return GMRFLib_SUCCESS;
}

/*!
  \brief Solve \f$Lx=b\f$ for indices in an interval
*/
int GMRFLib_solve_l_sparse_matrix_special(double *rhs, GMRFLib_sm_fact_tp * sm_fact, GMRFLib_graph_tp * graph, int findx, int toindx, int remapped)
{
	/*
	 * rhs in real world, bchol in mapped world. solve Lx=b backward only from rhs[findx] up to rhs[toindx]. note that
	 * findx and toindx is in mapped world. if remapped, do not remap/remap-back the rhs before solving.
	 * 
	 * this routine is called to many times and the work is not that much, to justify GMRFLib_ENTER_ROUTINE; 
	 */
	switch (sm_fact->smtp) {
	case GMRFLib_SMTP_BAND:
		GMRFLib_EWRAP0(GMRFLib_solve_l_sparse_matrix_special_BAND(rhs, sm_fact->bchol, graph, sm_fact->remap, sm_fact->bandwidth, findx, toindx, remapped));
		break;
	case GMRFLib_SMTP_PROFILE:
		GMRFLib_EWRAP0(GMRFLib_solve_l_sparse_matrix_special_PROFILE());
		break;
	case GMRFLib_SMTP_TAUCS:
		GMRFLib_EWRAP0(GMRFLib_solve_l_sparse_matrix_special_TAUCS(rhs, sm_fact->L, graph, sm_fact->remap, findx, toindx, remapped));
		break;
	default:
		GMRFLib_ERROR(GMRFLib_ESNH);
		break;
	}

	return GMRFLib_SUCCESS;
}

/*!
  \brief Compute the log determininant of \f$Q\f$
*/
int GMRFLib_log_determinant(double *logdet, GMRFLib_sm_fact_tp * sm_fact, GMRFLib_graph_tp * graph)
{
	switch (sm_fact->smtp) {
	case GMRFLib_SMTP_BAND:
		GMRFLib_EWRAP0(GMRFLib_log_determinant_BAND(logdet, sm_fact->bchol, graph, sm_fact->bandwidth));
		break;
	case GMRFLib_SMTP_PROFILE:
		GMRFLib_EWRAP0(GMRFLib_log_determinant_PROFILE());
		break;
	case GMRFLib_SMTP_TAUCS:
		GMRFLib_EWRAP0(GMRFLib_log_determinant_TAUCS(logdet, sm_fact->L));
		break;
	default:
		GMRFLib_ERROR(GMRFLib_ESNH);
		break;
	}
	return GMRFLib_SUCCESS;
}

/*!
  \brief Compute conditional mean and standard deviation of \f$x[i]\f$ conditioned on {\f$x[j]\f$}
    for \f$j>i\f$
*/
int GMRFLib_comp_cond_meansd(double *cmean, double *csd, int indx, double *x, int remapped, GMRFLib_sm_fact_tp * sm_fact, GMRFLib_graph_tp * graph)
{
	switch (sm_fact->smtp) {
	case GMRFLib_SMTP_BAND:
		GMRFLib_EWRAP1(GMRFLib_comp_cond_meansd_BAND(cmean, csd, indx, x, remapped, sm_fact->bchol, graph, sm_fact->remap, sm_fact->bandwidth));
		break;
	case GMRFLib_SMTP_PROFILE:
		GMRFLib_EWRAP1(GMRFLib_comp_cond_meansd_PROFILE());
		break;
	case GMRFLib_SMTP_TAUCS:
		GMRFLib_EWRAP1(GMRFLib_comp_cond_meansd_TAUCS(cmean, csd, indx, x, remapped, sm_fact->L, graph, sm_fact->remap));
		break;
	default:
		GMRFLib_ERROR(GMRFLib_ESNH);
		break;
	}

	return GMRFLib_SUCCESS;
}

/*!
  \brief Produce a bitmap of the Cholesky triangle in the portable bitmap (pbm) format
*/
int GMRFLib_bitmap_factorisation(const char *filename_body, GMRFLib_sm_fact_tp * sm_fact, GMRFLib_graph_tp * graph)
{
	switch (sm_fact->smtp) {
	case GMRFLib_SMTP_BAND:
		GMRFLib_EWRAP1(GMRFLib_bitmap_factorisation_BAND(filename_body, sm_fact->bchol, graph, sm_fact->remap, sm_fact->bandwidth));
		break;
	case GMRFLib_SMTP_PROFILE:
		GMRFLib_EWRAP1(GMRFLib_bitmap_factorisation_PROFILE());
		break;
	case GMRFLib_SMTP_TAUCS:
		GMRFLib_EWRAP1(GMRFLib_bitmap_factorisation_TAUCS(filename_body, sm_fact->L));
		break;
	default:
		GMRFLib_ERROR(GMRFLib_ESNH);
		break;
	}
	return GMRFLib_SUCCESS;
}

/*!
  \brief Wrapper for computing the (structural) inverse of \c Q.
*/
int GMRFLib_compute_Qinv(void *problem, int storage)
{
	GMRFLib_problem_tp *p = (GMRFLib_problem_tp *) problem;

	switch (p->sub_sm_fact.smtp) {
	case GMRFLib_SMTP_BAND:
		GMRFLib_EWRAP0(GMRFLib_compute_Qinv_BAND(p, storage));
		break;
	case GMRFLib_SMTP_PROFILE:
		GMRFLib_EWRAP0(GMRFLib_compute_Qinv_PROFILE());
		break;
	case GMRFLib_SMTP_TAUCS:
		GMRFLib_EWRAP0(GMRFLib_compute_Qinv_TAUCS(p, storage));
		break;
	default:
		GMRFLib_ERROR(GMRFLib_ESNH);
		break;
	}
	return GMRFLib_SUCCESS;
}

/*!
  \brief Return \c GMRFLib_TRUE or \c GMRFLib_FALSE if smtp is valid
*/
int GMRFLib_valid_smtp(int smtp)
{
	if ((smtp == GMRFLib_SMTP_BAND) || (smtp == GMRFLib_SMTP_PROFILE) || (smtp == GMRFLib_SMTP_TAUCS)) {
		return GMRFLib_TRUE;
	} else {
		return GMRFLib_FALSE;
	}
}

/*! 
  \brief Return the name of a reordering
 */
const char *GMRFLib_reorder_name(GMRFLib_reorder_tp r)
{
	switch (r) {
	case GMRFLib_REORDER_DEFAULT:
		return "default";
	case GMRFLib_REORDER_IDENTITY:
		return "identity";
	case GMRFLib_REORDER_BAND:
		return "band";
	case GMRFLib_REORDER_METIS:
		return "metis";
	case GMRFLib_REORDER_GENMMD:
		return "genmmd";
	case GMRFLib_REORDER_AMD:
		return "amd";
	case GMRFLib_REORDER_MD:
		return "md";
	case GMRFLib_REORDER_MMD:
		return "mmd";
	default:
		fprintf(stderr, "\n\t*** ERROR *** Reordering [%d] not defined.\n", r);
		GMRFLib_ASSERT_RETVAL(0 == 1, GMRFLib_EPARAMETER, "(unknown reording)");
	}

	return "(unknown reording)";
}

/*! 
  \brief Return the id of the reordering from the name. 
 */
int GMRFLib_reorder_id(const char *name)
{
	if (!strcasecmp(name, "default"))
		return GMRFLib_REORDER_DEFAULT;
	else if (!strcasecmp(name, "identity"))
		return GMRFLib_REORDER_IDENTITY;
	else if (!strcasecmp(name, "band"))
		return GMRFLib_REORDER_BAND;
	else if (!strcasecmp(name, "metis"))
		return GMRFLib_REORDER_METIS;
	else if (!strcasecmp(name, "genmmd"))
		return GMRFLib_REORDER_GENMMD;
	else if (!strcasecmp(name, "amd"))
		return GMRFLib_REORDER_AMD;
	else if (!strcasecmp(name, "md"))
		return GMRFLib_REORDER_MD;
	else if (!strcasecmp(name, "mmd"))
		return GMRFLib_REORDER_MMD;
	else if (!strcasecmp(name, "auto"))
		return -1;				       /* THIS IS SPECIAL */
	else {
		fprintf(stderr, "\n\t*** ERROR *** Reordering [%s] not defined.\n", name);
		GMRFLib_ASSERT_RETVAL(0 == 1, GMRFLib_EPARAMETER, -1);
	}

	return -1;
}