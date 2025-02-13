
#include <stdio.h>
#include <math.h>
#include "nr_util.h"
#include "ad_globl.h"
#include "ad_frsnl.h"
#include "iad_type.h"
#include "iad_util.h"
#include "iad_calc.h"
#include "iad_find.h"
#include "iad_pub.h"
#include "iad_io.h"
#include "mc_lost.h"



void
Inverse_RT (struct measure_type m, struct invert_type *r)
{
  if (0 && Debug (DEBUG_LOST_LIGHT))
    {
      fprintf (stderr, "** Inverse_RT (%d spheres) **\n", m.num_spheres);
      fprintf (stderr, "    M_R      = %8.5f, MT       = %8.5f\n", m.m_r,
	       m.m_t);
      fprintf (stderr, "    UR1 lost = %8.5f, UT1 lost = %8.5f\n", m.ur1_lost,
	       m.ut1_lost);
    }

  r->found = FALSE;

  if (r->search == FIND_AUTO)
    r->search = determine_search (m, *r);

  if (r->search == FIND_B_WITH_NO_ABSORPTION)
    {
      r->default_a = 1;
      r->search = FIND_B;
    }

  if (r->search == FIND_B_WITH_NO_SCATTERING)
    {
      r->default_a = 0;
      r->search = FIND_B;
    }


  r->error = measure_OK (m, *r);

  if (r->method.quad_pts < 4)
    r->error = IAD_QUAD_PTS_NOT_VALID;

  if (r->error != IAD_NO_ERROR)
    return;




  switch (r->search)
    {
    case FIND_A:
      U_Find_A (m, r);
      break;
    case FIND_B:
      U_Find_B (m, r);
      break;
    case FIND_G:
      U_Find_G (m, r);
      break;
    case FIND_Ba:
      U_Find_Ba (m, r);
      break;
    case FIND_Bs:
      U_Find_Bs (m, r);
      break;

    case FIND_AB:
      U_Find_AB (m, r);
      break;
    case FIND_AG:
      U_Find_AG (m, r);
      break;
    case FIND_BG:
      U_Find_BG (m, r);
      break;
    case FIND_BsG:
      U_Find_BsG (m, r);
      break;
    case FIND_BaG:
      U_Find_BaG (m, r);
      break;
    }
  if (r->iterations == IAD_MAX_ITERATIONS)
    r->error = IAD_TOO_MANY_ITERATIONS;


  if (r->final_distance <= r->tolerance)
    r->found = TRUE;
}




int
measure_OK (struct measure_type m, struct invert_type r)
{
  double ru, tu;

  if (m.num_spheres != 2)
    {


      if (m.m_t < 0)
	return IAD_MT_TOO_SMALL;

      Sp_mu_RT_Flip (m.flip_sample, r.slab.n_top_slide, r.slab.n_slab,
		     r.slab.n_bottom_slide, r.slab.b_top_slide, 0,
		     r.slab.b_bottom_slide, r.slab.cos_angle, &ru, &tu);

      if (m.num_spheres == 0 && m.m_t > tu)
	{
	  fprintf (stderr, "ntop=%7.5f, nslab=%7.5f, nbottom=%7.5f\n",
		   r.slab.n_top_slide, r.slab.n_slab, r.slab.n_bottom_slide);
	  fprintf (stderr, "tu_max=%7.5f, m_t=%7.5f, t_std=%7.5f\n", tu,
		   m.m_t, m.rstd_t);
	  return IAD_MT_TOO_BIG;
	}



      {
	double mr, mt;
	Calculate_Minimum_MR (m, r, &mr, &mt);


	if (r.search == FIND_A || r.search == FIND_G || r.search == FIND_B ||
	    r.search == FIND_Bs || r.search == FIND_Ba)
	  {
	    if (m.m_r < mr && m.m_t <= 0)
	      return IAD_MR_TOO_SMALL;
	  }
	else
	  {

	    if (r.default_a == UNINITIALIZED || r.default_a > 0)
	      {
		if (m.m_r < mr)
		  return IAD_MR_TOO_SMALL;
	      }
	  }
      }


    }
  else
    {
      int error = MinMax_MR_MT (m, r);
      if (error != IAD_NO_ERROR)
	return error;
    }



  if (m.m_u < 0)
    return IAD_MU_TOO_SMALL;

  if (m.m_u > m.m_t)
    return IAD_MU_TOO_BIG;




  if (m.num_spheres != 0)
    {


      if (m.as_r < 0 || m.as_r >= 0.2)
	return IAD_AS_NOT_VALID;

      if (m.ad_r < 0 || m.ad_r >= 0.2)
	return IAD_AD_NOT_VALID;

      if (m.ae_r < 0 || m.ae_r >= 0.2)
	return IAD_AE_NOT_VALID;

      if (m.rw_r < 0 || m.rw_r > 1.0)
	return IAD_RW_NOT_VALID;

      if (m.rd_r < 0 || m.rd_r > 1.0)
	return IAD_RD_NOT_VALID;

      if (m.rstd_r < 0 || m.rstd_r > 1.0)
	return IAD_RSTD_NOT_VALID;

      if (m.rstd_t < 0 || m.rstd_t > 1.0)
	return IAD_TSTD_NOT_VALID;

      if (m.f_r < 0 || m.f_r > 1)
	return IAD_F_NOT_VALID;



      if (m.as_t < 0 || m.as_t >= 0.2)
	return IAD_AS_NOT_VALID;

      if (m.ad_t < 0 || m.ad_t >= 0.2)
	return IAD_AD_NOT_VALID;

      if (m.ae_t < 0 || m.ae_t >= 0.2)
	return IAD_AE_NOT_VALID;

      if (m.rw_t < 0 || m.rw_r > 1.0)
	return IAD_RW_NOT_VALID;

      if (m.rd_t < 0 || m.rd_t > 1.0)
	return IAD_RD_NOT_VALID;

      if (m.rstd_t < 0 || m.rstd_t > 1.0)
	return IAD_TSTD_NOT_VALID;

      if (m.f_t < 0 || m.f_t > 1)
	return IAD_F_NOT_VALID;


    }

  return IAD_NO_ERROR;
}




search_type
determine_search (struct measure_type m, struct invert_type r)
{
  double rt, tt, rd, td, tc, rc;
  int search = 0;
  int independent = m.num_measures;

  if (Debug (DEBUG_SEARCH))
    {
      fprintf (stderr, "\n*** Determine_Search()\n");
      fprintf (stderr, "    starting with %d measurement(s)\n",
	       m.num_measures);
      fprintf (stderr, "    m_r=%.5f\n", m.m_r);
      fprintf (stderr, "    m_t=%.5f\n", m.m_t);
    }

  Estimate_RT (m, r, &rt, &tt, &rd, &rc, &td, &tc);

  if (m.m_u == 0 && independent == 3)
    {
      if (Debug (DEBUG_SEARCH))
	fprintf (stderr, "    no information in tc\n");
      independent--;
    }

  if (rd == 0 && independent == 2)
    {
      if (Debug (DEBUG_SEARCH))
	fprintf (stderr, "    no information in rd\n");
      independent--;
    }

  if (td == 0 && independent == 2)
    {
      if (Debug (DEBUG_SEARCH))
	fprintf (stderr, "    no information in td\n");
      independent--;
    }

  if (independent == 1)
    {

      if (r.default_a != UNINITIALIZED)
	{
	  if (r.default_a == 0)
	    search = FIND_B_WITH_NO_SCATTERING;
	  else if (r.default_a == 1)
	    search = FIND_B_WITH_NO_ABSORPTION;
	  else if (tt == 0)
	    search = FIND_G;
	  else
	    search = FIND_B;
	}
      else if (r.default_b != UNINITIALIZED)
	search = FIND_A;

      else if (r.default_bs != UNINITIALIZED)
	search = FIND_Ba;

      else if (r.default_ba != UNINITIALIZED)
	search = FIND_Bs;

      else if (td == 0)
	search = FIND_A;

      else if (rd == 0)
	search = FIND_B_WITH_NO_SCATTERING;

      else
	search = FIND_B_WITH_NO_ABSORPTION;


    }

  else if (independent == 2)
    {

      if (r.default_a != UNINITIALIZED)
	{

	  if (r.default_a == 0)
	    search = FIND_B;
	  else if (r.default_g != UNINITIALIZED)
	    search = FIND_B;
	  else
	    search = FIND_BG;

	}
      else if (r.default_b != UNINITIALIZED)
	{

	  if (r.default_g != UNINITIALIZED)
	    search = FIND_A;
	  else
	    search = FIND_AG;

	}
      else if (r.default_ba != UNINITIALIZED)
	{

	  if (r.default_g != UNINITIALIZED)
	    search = FIND_Bs;
	  else
	    search = FIND_BsG;

	}
      else if (r.default_bs != UNINITIALIZED)
	{

	  if (r.default_g != UNINITIALIZED)
	    search = FIND_Ba;
	  else
	    search = FIND_BaG;

	}
      else if (rt + tt > 1 && 0 && m.num_spheres != 2)
	search = FIND_B_WITH_NO_ABSORPTION;

      else
	search = FIND_AB;



    }


  else
    {
      search = FIND_AG;
    }

  if (Debug (DEBUG_SEARCH))
    {
      fprintf (stderr, "    independent measurements = %3d\n", independent);
      fprintf (stderr, "    m_r=%8.5f m_t=%8.5f (rd = %8.5f td=%8.5f)\n",
	       m.m_r, m.m_t, rd, td);
      if (search == FIND_A)
	fprintf (stderr, "    search = FIND_A\n");
      if (search == FIND_B)
	fprintf (stderr, "    search = FIND_B\n");
      if (search == FIND_AB)
	fprintf (stderr, "    search = FIND_AB\n");
      if (search == FIND_AG)
	fprintf (stderr, "    search = FIND_AG\n");
      if (search == FIND_AUTO)
	fprintf (stderr, "    search = FIND_AUTO\n");
      if (search == FIND_BG)
	fprintf (stderr, "    search = FIND_BG\n");
      if (search == FIND_BaG)
	fprintf (stderr, "    search = FIND_BaG\n");
      if (search == FIND_BsG)
	fprintf (stderr, "    search = FIND_BsG\n");
      if (search == FIND_Ba)
	fprintf (stderr, "    search = FIND_Ba\n");
      if (search == FIND_Bs)
	fprintf (stderr, "    search = FIND_Bs\n");
      if (search == FIND_G)
	fprintf (stderr, "    search = FIND_G\n");
      if (search == FIND_B_WITH_NO_ABSORPTION)
	fprintf (stderr, "    search = FIND_B_WITH_NO_ABSORPTION\n");
      if (search == FIND_B_WITH_NO_SCATTERING)
	fprintf (stderr, "    search = FIND_B_WITH_NO_SCATTERING\n");
    }

  return search;
}





void
Initialize_Result (struct measure_type m, struct invert_type *r)
{

  r->a = 0.0;
  r->b = 0.0;
  r->g = 0.0;


  r->found = FALSE;
  r->tolerance = 0.0001;
  r->MC_tolerance = 0.01;
  r->search = FIND_AUTO;
  r->metric = RELATIVE;
  r->final_distance = 10;
  r->iterations = 0;
  r->error = IAD_NO_ERROR;


  r->default_a = UNINITIALIZED;
  r->default_b = UNINITIALIZED;
  r->default_g = UNINITIALIZED;
  r->default_ba = UNINITIALIZED;
  r->default_bs = UNINITIALIZED;
  r->default_mua = UNINITIALIZED;
  r->default_mus = UNINITIALIZED;




  r->slab.a = 0.5;
  r->slab.b = 1.0;
  r->slab.g = 0;
  r->slab.phase_function = HENYEY_GREENSTEIN;
  r->slab.n_slab = m.slab_index;
  r->slab.n_top_slide = m.slab_top_slide_index;
  r->slab.n_bottom_slide = m.slab_bottom_slide_index;
  r->slab.b_top_slide = m.slab_top_slide_b;
  r->slab.b_bottom_slide = m.slab_bottom_slide_b;
  r->slab.cos_angle = m.slab_cos_angle;

  r->method.a_calc = 0.5;
  r->method.b_calc = 1;
  r->method.g_calc = 0.5;
  r->method.quad_pts = 8;
  r->method.b_thinnest = 1.0 / 32.0;


}




void
Initialize_Measure (struct measure_type *m)
{
  double default_sphere_d = 8.0 * 25.4;
  double default_sample_d = 0.0 * 25.4;
  double default_detector_d = 0.1 * 25.4;
  double default_entrance_d = 0.5 * 25.4;
  double sphere = default_sphere_d * default_sphere_d;

  m->slab_index = 1.0;
  m->slab_top_slide_index = 1.0;
  m->slab_top_slide_b = 0.0;
  m->slab_top_slide_thickness = 0.0;
  m->slab_bottom_slide_index = 1.0;
  m->slab_bottom_slide_b = 0.0;
  m->slab_bottom_slide_thickness = 0.0;
  m->slab_thickness = 1.0;
  m->slab_cos_angle = 1.0;

  m->num_spheres = 0;
  m->num_measures = 1;
  m->method = UNKNOWN;

  m->fraction_of_rc_in_mr = 1.0;
  m->fraction_of_tc_in_mt = 1.0;

  m->flip_sample = 0;

  m->m_r = 0.0;
  m->m_t = 0.0;
  m->m_u = 0.0;

  m->d_sphere_r = default_sphere_d;
  m->as_r = default_sample_d * default_sample_d / sphere;
  m->ad_r = default_detector_d * default_detector_d / sphere;
  m->ae_r = default_entrance_d * default_entrance_d / sphere;
  m->aw_r = 1.0 - m->as_r - m->ad_r - m->ae_r;
  m->rd_r = 0.0;
  m->rw_r = 1.0;
  m->rstd_r = 1.0;
  m->f_r = 0.0;

  m->d_sphere_t = default_sphere_d;
  m->as_t = m->as_r;
  m->ad_t = m->ad_r;
  m->ae_t = m->ae_r;
  m->aw_t = m->aw_r;
  m->rd_t = 0.0;
  m->rw_t = 1.0;
  m->rstd_t = 1.0;
  m->f_t = 0.0;

  m->lambda = 0.0;
  m->d_beam = 0.0;
  m->ur1_lost = 0;
  m->uru_lost = 0;
  m->ut1_lost = 0;
  m->utu_lost = 0;
}




void
ez_Inverse_RT (double n, double nslide, double UR1, double UT1, double Tc,
	       double *a, double *b, double *g, int *error)
{
  struct measure_type m;
  struct invert_type r;
  *a = 0;
  *b = HUGE_VAL;
  *g = 0;

  Initialize_Measure (&m);

  m.slab_index = n;
  m.slab_top_slide_index = nslide;
  m.slab_bottom_slide_index = nslide;
  m.slab_cos_angle = 1.0;

  m.num_measures = 3;
  if (UT1 == 0)
    m.num_measures--;
  if (Tc == 0)
    m.num_measures--;

  m.m_r = UR1;
  m.m_t = UT1;
  m.m_u = Tc;

  Initialize_Result (m, &r);
  r.method.quad_pts = 8;

  Inverse_RT (m, &r);

  *error = r.error;
  if (r.error == IAD_NO_ERROR)
    {
      *a = r.a;
      *b = r.b;
      *g = r.g;
    }
}




void
Spheres_Inverse_RT (double *setup,
		    double *analysis,
		    double *sphere_r,
		    double *sphere_t, double *measurements, double *results)
{
  struct measure_type m;
  struct invert_type r;
  long num_photons;
  double ur1, ut1, uru, utu;
  int i, mc_runs = 1;

  Initialize_Measure (&m);


  {
    double d_sample_r, d_entrance_r, d_detector_r;
    double d_sample_t, d_entrance_t, d_detector_t;

    m.slab_index = setup[0];
    m.slab_top_slide_index = setup[1];
    m.slab_thickness = setup[2];
    m.slab_top_slide_thickness = setup[3];
    m.d_beam = setup[4];
    m.rstd_r = setup[5];
    m.num_spheres = (int) setup[6];

    m.d_sphere_r = setup[7];
    d_sample_r = setup[8];
    d_entrance_r = setup[9];
    d_detector_r = setup[10];
    m.rw_r = setup[11];

    m.d_sphere_t = setup[12];
    d_sample_t = setup[13];
    d_entrance_t = setup[14];
    d_detector_t = setup[15];
    m.rw_t = setup[16];

    r.default_g = setup[17];
    num_photons = (long) setup[18];

    m.as_r = (d_sample_r / m.d_sphere_r) * (d_sample_r / m.d_sphere_r);
    m.ae_r = (d_entrance_r / m.d_sphere_r) * (d_entrance_r / m.d_sphere_r);
    m.ad_r = (d_detector_r / m.d_sphere_r) * (d_detector_r / m.d_sphere_r);
    m.aw_r = 1.0 - m.as_r - m.ae_r - m.ad_r;
    m.as_t = (d_sample_t / m.d_sphere_t) * (d_sample_t / m.d_sphere_t);
    m.ae_t = (d_entrance_t / m.d_sphere_t) * (d_entrance_t / m.d_sphere_t);
    m.ad_t = (d_detector_t / m.d_sphere_t) * (d_detector_t / m.d_sphere_t);
    m.aw_t = 1.0 - m.as_t - m.ae_t - m.ad_t;

    m.slab_bottom_slide_index = m.slab_top_slide_index;
    m.slab_bottom_slide_thickness = m.slab_top_slide_thickness;

    fprintf (stderr, "**** executing FIXME ****/n");
    m.slab_cos_angle = 1.0;

  }



  m.as_r = sphere_r[0];
  m.ae_r = sphere_r[1];
  m.ad_r = sphere_r[2];
  m.rw_r = sphere_r[3];
  m.rd_r = sphere_r[4];
  m.rstd_r = sphere_r[5];
  m.f_r = sphere_r[7];



  m.as_t = sphere_t[0];
  m.ae_t = sphere_t[1];
  m.ad_t = sphere_t[2];
  m.rw_t = sphere_t[3];
  m.rd_t = sphere_t[4];
  m.rstd_t = sphere_t[5];
  m.f_t = sphere_t[7];



  m.m_r = measurements[0];
  m.m_t = measurements[1];
  m.m_u = measurements[2];

  m.num_measures = 3;
  if (m.m_t == 0)
    m.num_measures--;
  if (m.m_u == 0)
    m.num_measures--;



  Initialize_Result (m, &r);
  results[0] = 0;
  results[1] = 0;
  results[2] = 0;



  r.method.quad_pts = (int) analysis[0];
  mc_runs = (int) analysis[1];



  Inverse_RT (m, &r);
  for (i = 0; i < mc_runs; i++)
    {
      MC_Lost (m, r, num_photons, &ur1, &ut1, &uru, &utu,
	       &m.ur1_lost, &m.ut1_lost, &m.uru_lost, &m.utu_lost);
      Inverse_RT (m, &r);
    }

  if (r.error == IAD_NO_ERROR)
    {
      results[0] = (1 - r.a) * r.b / m.slab_thickness;
      results[1] = (r.a) * r.b / m.slab_thickness;
      results[2] = r.g;
    }

  results[3] = r.error;
}




void
Spheres_Inverse_RT2 (double *sample,
		     double *illumination,
		     double *sphere_r,
		     double *sphere_t,
		     double *analysis,
		     double *measurement, double *a, double *b, double *g)
{
  struct measure_type m;
  struct invert_type r;
  long num_photons;
  double ur1, ut1, uru, utu;
  int i, mc_runs = 1;

  Initialize_Measure (&m);


  m.slab_index = sample[0];
  m.slab_top_slide_index = sample[1];
  m.slab_bottom_slide_index = sample[2];
  m.slab_thickness = sample[3];
  m.slab_top_slide_thickness = sample[4];
  m.slab_bottom_slide_thickness = sample[5];
  m.slab_top_slide_thickness = 0;
  m.slab_bottom_slide_thickness = 0;



  m.d_beam = illumination[0];




  m.num_spheres = illumination[5];



  {
    double d_sample_r, d_entrance_r, d_detector_r;

    m.d_sphere_r = sphere_r[0];
    d_sample_r = sphere_r[1];
    d_entrance_r = sphere_r[2];
    d_detector_r = sphere_r[3];
    m.rw_r = sphere_r[4];
    m.rd_r = sphere_r[5];

    m.as_r = (d_sample_r / m.d_sphere_r) * (d_sample_r / m.d_sphere_r);
    m.ae_r = (d_entrance_r / m.d_sphere_r) * (d_entrance_r / m.d_sphere_r);
    m.ad_r = (d_detector_r / m.d_sphere_r) * (d_detector_r / m.d_sphere_r);
    m.aw_r = 1.0 - m.as_r - m.ae_r - m.ad_r;
  }



  {
    double d_sample_t, d_entrance_t, d_detector_t;

    m.d_sphere_t = sphere_t[0];
    d_sample_t = sphere_t[1];
    d_entrance_t = sphere_t[2];
    d_detector_t = sphere_t[3];
    m.rw_t = sphere_t[4];
    m.rd_t = sphere_t[5];

    m.as_t = (d_sample_t / m.d_sphere_t) * (d_sample_t / m.d_sphere_t);
    m.ae_t = (d_entrance_t / m.d_sphere_t) * (d_entrance_t / m.d_sphere_t);
    m.ad_t = (d_detector_t / m.d_sphere_t) * (d_detector_t / m.d_sphere_t);
    m.aw_t = 1.0 - m.as_t - m.ae_t - m.ad_t;
  }



  r.method.quad_pts = (int) analysis[0];
  mc_runs = (int) analysis[1];
  num_photons = (long) analysis[2];




  m.rstd_r = measurement[0];
  m.m_r = measurement[1];
  m.m_t = measurement[2];
  m.m_u = measurement[3];

  m.num_measures = 3;
  if (m.m_t == 0)
    m.num_measures--;
  if (m.m_u == 0)
    m.num_measures--;

  Initialize_Result (m, &r);

  Inverse_RT (m, &r);
  for (i = 0; i < mc_runs; i++)
    {
      MC_Lost (m, r, num_photons, &ur1, &ut1, &uru, &utu,
	       &m.ur1_lost, &m.ut1_lost, &m.uru_lost, &m.utu_lost);
      Inverse_RT (m, &r);
    }

  if (r.error == IAD_NO_ERROR)
    {
      *a = r.a;
      *b = r.b;
      *g = r.g;
    }
}




void
Calculate_MR_MT (struct measure_type m,
		 struct invert_type r,
		 int include_MC, double *M_R, double *M_T)
{
  double distance, ur1, ut1, uru, utu;
  struct measure_type old_mm;
  struct invert_type old_rr;

  if (include_MC && m.num_spheres > 0)
    MC_Lost (m, r, -2000, &ur1, &ut1, &uru, &utu,
	     &(m.ur1_lost), &(m.ut1_lost), &(m.uru_lost), &(m.utu_lost));

  Get_Calc_State (&old_mm, &old_rr);
  Set_Calc_State (m, r);

  Calculate_Distance (M_R, M_T, &distance);

  Set_Calc_State (old_mm, old_rr);
}




int
MinMax_MR_MT (struct measure_type m, struct invert_type r)
{
  double distance, measured_m_r, min_possible_m_r, max_possible_m_r, temp_m_t;

  if (m.m_r < 0)
    return IAD_MR_TOO_SMALL;
  if (m.m_r * m.rstd_r > 1)
    return IAD_MR_TOO_BIG;
  if (m.m_t < 0)
    return IAD_MT_TOO_SMALL;
  if (m.m_t == 0)
    return IAD_NO_ERROR;

  measured_m_r = m.m_r;

  m.m_r = 0;
  r.search = FIND_B;

  r.default_a = 0;
  U_Find_B (m, &r);
  Calculate_Distance (&min_possible_m_r, &temp_m_t, &distance);
  if (measured_m_r < min_possible_m_r)
    return IAD_MR_TOO_SMALL;

  r.default_a = 1.0;
  U_Find_B (m, &r);
  Calculate_Distance (&max_possible_m_r, &temp_m_t, &distance);
  if (measured_m_r > max_possible_m_r)
    return IAD_MR_TOO_BIG;

  return IAD_NO_ERROR;
}




void
Calculate_Minimum_MR (struct measure_type m,
		      struct invert_type r, double *mr, double *mt)
{
  if (r.default_b == UNINITIALIZED)
    if (r.slab.n_slab > 1.0)
      r.slab.b = HUGE_VAL;
    else
      r.slab.b = 1e-5;
  else
    r.slab.b = r.default_b;

  if (r.default_a == UNINITIALIZED)
    r.slab.a = 0;
  else
    r.slab.a = r.default_a;

  if (r.default_g == UNINITIALIZED)
    r.slab.g = 0.0;
  else
    r.slab.g = r.default_g;

  if (r.search == FIND_G)
    r.slab.a = 0;

  r.a = r.slab.a;
  r.b = r.slab.b;
  r.g = r.slab.g;

  Calculate_MR_MT (m, r, 0, mr, mt);
  *mt = 0;
}
