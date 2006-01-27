//----------------------------  grid_transform.cc  ---------------------------
//    $Id$
//    Version: $Name$ 
//
//    Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  grid_transform.cc  ---------------------------



#include "../tests.h"
#include <base/logstream.h>
#include <grid/tria.h>
#include <grid/tria_accessor.h>
#include <grid/tria_boundary_lib.h>
#include <grid/grid_generator.h>
#include <grid/grid_out.h>
#include <fe/fe_q.h>
#include <numerics/data_out.h>
#include <lac/vector.h>

#include <fstream>
#include <iostream>


int main ()
{
  const unsigned int dim=3;
  Point<dim> origin;
  CylinderBoundary<dim> boundary;
  Triangulation<dim> tria;
  tria.set_boundary(0, boundary);
  GridGenerator::cylinder(tria, 1, .7);
  tria.refine_global(2);
  
				   // build up a map of vertex indices
				   // of boundary vertices to the new
				   // boundary points
  std::map<unsigned int,Point<dim> > new_points;
  
  Triangulation<dim>::active_cell_iterator cell=tria.begin_active(),
					   endc=tria.end();

  for (unsigned int i=0; i<4; ++i)
    {
      for (cell=tria.begin_active(); cell!=endc; ++cell)
	if ((cell->center()[0]-.2)*(cell->center()[0]-.2) +
	    (cell->center()[2]-cell->center()[1]/4)*(cell->center()[2]-cell->center()[1]/4)
	    < cell->diameter() * cell->diameter())
	  cell->set_refine_flag ();
      tria.execute_coarsening_and_refinement();
    }
  for (cell=tria.begin_active(); cell!=endc; ++cell)
    if ((cell->center()[0]-.2)*(cell->center()[0]-.2) +
	(cell->center()[2]-cell->center()[1]/4)*(cell->center()[2]-cell->center()[1]/4)
	< cell->diameter() * cell->diameter())
      cell->set_refine_flag ();
    else
      cell->set_coarsen_flag ();
  tria.execute_coarsening_and_refinement();

  
  Triangulation<dim>::face_iterator face;
  for (cell=tria.begin_active(); cell!=endc; ++cell)
    for (unsigned int face_no=0; face_no<GeometryInfo<dim>::faces_per_cell; ++face_no)
      {
	face=cell->face(face_no);
	if (face->at_boundary())
	  for (unsigned int vertex_no=0;
	       vertex_no<GeometryInfo<dim>::vertices_per_face; ++vertex_no)
	    {
	      const Point<dim> &old_vertex=face->vertex(vertex_no);

	      Point<dim> new_vertex (old_vertex[0],
				     old_vertex[1],
				     (old_vertex[0] <= 0 ?
				      old_vertex[2]/2 :
				      old_vertex[2]/(2+4*old_vertex[0]*old_vertex[0])));
	      
	      new_points[face->vertex_index(vertex_no)] = new_vertex;
	    }  
      }

  GridGenerator::laplace_transformation (tria, new_points);

    
  
  if (false)
    {
      GridOut grid_out;
      std::ofstream out("grid_transform_3d/output");
      grid_out.write_gnuplot(tria, out);
    }
  else
    {
      FE_Q<dim> fe(1);
      DoFHandler<dim> dof_handler (tria);
      dof_handler.distribute_dofs(fe);
      Vector<double> v(dof_handler.n_dofs());
      DataOut<dim> data_out;
      data_out.attach_dof_handler (dof_handler);
      data_out.add_data_vector (v, "v");
      data_out.build_patches();
      std::ofstream out("grid_transform_3d/output.gmv");
      data_out.write_gmv(out);
    }
  
  tria.clear();
  
  return 0;
}
