#include "fdebug.h"

module vtkFortranSupermesh


  use elements
  use shape_Functions
  use fields
  use iso_c_binding
  use unify_meshes_module




  implicit none

  integer, parameter :: VTK_VERTEX=1, VTK_LINE=3, VTK_TRIANGLE=5,&
       VTK_TETRA=10

#include "supermesh_femtools/petsc_legacy.h"

contains

  function get_shape_from_vtk_type(vtk_type) result (shape)
    integer, intent(in) :: vtk_type
    type(element_type) :: shape
    type(quadrature_type):: quad

    integer :: loc, dim, degree

    select case(vtk_type)
    case(VTK_VERTEX)
       loc = 1
       dim = 0
       degree = 1
    case(VTK_LINE)
       loc = 2
       dim = 1
       degree = 1
    case(VTK_TRIANGLE)
       loc = 3
       dim = 2
       degree = 1
    case(VTK_TETRA)
       loc = 3
       dim = 3
       degree = 1
    end select

    quad = make_quadrature(loc, dim, degree = 1)
    shape = make_element_shape(loc, dim, degree, quad)

  end function get_shape_from_vtk_type


  function position_field_from_vtk(X,C,vtk_type) result(position)

    real, dimension(:,:), intent(in), target :: X
    integer (kind=c_long_long), dimension(:), intent(in) :: C
    integer, intent(in) :: vtk_type
    type(vector_field) :: position

    type(mesh_type) :: mesh
    type(element_type) :: shape

    integer :: ele, i

    shape = get_shape_from_vtk_type(vtk_type)

    call allocate(mesh,size(X,2),size(C)/(shape%loc+1),shape,"VTKMesh")

    do ele=1, size(C)/(shape%loc+1)
       i=(ele-1)*(shape%loc+1)+1
       assert(C(i)==shape%loc)
       call set_ele_nodes(mesh, ele,int(C(i+1:i+shape%loc))+1)
    end do

    call allocate(position,shape%dim,mesh,"Coordinate")
    position%val(:,:)= X(1:shape%dim,:)

  end function position_field_from_vtk

  subroutine recursive_supermesh(positionsA, positionsB, map_BA, supermesh_shape, start_ele, end_ele, supermesh)
    type(vector_field), intent(in) :: positionsA, positionsB
    type(ilist), dimension(:), intent(in) :: map_BA
    type(element_type), intent(in) :: supermesh_shape
    integer, intent(in) :: start_ele, end_ele
    type(vector_field), intent(out) :: supermesh

    integer, parameter :: blocksize = 4
    integer :: i

    type(vector_field) :: supermesh_tmp
    type(mesh_type) :: supermesh_mesh
    type(vector_field) :: supermesh_accum
    type(inode), pointer :: llnode

    integer :: ele_A, ele_B
    integer :: new_start, new_end, step

    integer :: nintersections, nintersections_max
    integer, dimension(:, :), allocatable :: intersection_parents
    type(vector_field), dimension(:), allocatable :: intersections
  

    nintersections_max = sum(map_BA%length)
    ewrite(2, "(a,i0)") "Maximum number of intersections: ", nintersections_max
    allocate(intersections(nintersections_max))
    allocate(intersection_parents(nintersections_max, 2))
    nintersections = 0

    call allocate(supermesh_mesh, 0, 0, supermesh_shape, "AccumulatedMesh")
    call allocate(supermesh, positionsA%dim, supermesh_mesh, "AccumulatedPositions")
    call deallocate(supermesh_mesh)

    do ele_B=start_ele,end_ele
       llnode => map_BA(ele_B)%firstnode
       do while(associated(llnode))
          ele_A = llnode%value
          supermesh_tmp = intersect_elements(positionsA, ele_A, ele_val(positionsB, ele_B), supermesh_shape)
          if(ele_count(supermesh_tmp) > 0) then
             nintersections = nintersections + 1
             intersections(nintersections) = supermesh_tmp
             intersection_parents(nintersections, :) = (/ele_A, ele_B/)
          else
             call deallocate(supermesh_tmp)
          end if
          llnode => llnode%next
       end do
    end do
    
    supermesh = unify_meshes(intersections(:nintersections))
    supermesh%name = "Coordinate"
    
  
  end subroutine recursive_supermesh

end module vtkFortranSupermesh

subroutine supermesh(X1,N_X1,C1,N_C1,&
                     X2,N_X2,C2,N_C2,&
                     nNodes,nEles,nEntries,pos_c) bind(C)
  use vtkFortranSupermesh
  use iso_c_binding
  use write_gmsh

  implicit none

  integer(kind=c_int), value :: N_X1, N_C1, N_X2, N_C2
  real(kind=c_double) :: X1(3,N_X1), X2(3,N_X2)
  integer (kind=c_long_long) :: C1(N_C1),  C2(N_C2)
  integer(kind=c_int) :: nNodes,nEles,nEntries
  type(c_ptr) :: pos_c

  type(vector_field) :: positionsA, positionsB
  type(ilist), dimension(:), allocatable :: map_BA
  real, dimension(:), allocatable :: tri_detwei
  integer :: ele_A, ele_B
  type(inode), pointer :: llnode
  type(vector_field) :: intersection
  type(element_type) :: supermesh_shape
  type(quadrature_type) :: supermesh_quad
  integer :: dim

  type(mesh_type) :: accum_mesh
  type(vector_field) ::  accum_positions_tmp
  type(vector_field), pointer ::accum_positions

  positionsA=position_field_from_vtk(X1,C1,VTK_TRIANGLE)
  positionsB=position_field_from_vtk(X2,C2,VTK_TRIANGLE)

  dim = positionsA%dim

  allocate(map_BA(ele_count(positionsB)))

  supermesh_quad = make_quadrature(vertices=dim+1, dim=dim, degree=5)
  supermesh_shape = make_element_shape(vertices=dim+1, dim=dim, degree=1, quad=supermesh_quad)
  allocate(tri_detwei(supermesh_shape%ngi))

  allocate(accum_positions)

  call allocate(accum_mesh, 0, 0, supermesh_shape, "AccumulatedMesh")
  call allocate(accum_positions, dim, accum_mesh, "AccumulatedPositions")

  map_BA = rtree_intersection_finder(positionsB, positionsA)
  call intersector_set_dimension(dim)
  call intersector_set_exactness(.false.)

  ! inputs: positionsB, map_BA, positionsA, supermesh_shape
  ! output: the supermesh!
  call recursive_supermesh(positionsA, positionsB, map_BA, supermesh_shape, 1, ele_count(positionsB), accum_positions)

  nNodes=node_count(accum_positions)
  nEles=element_count(accum_positions)
  nEntries=(ele_loc(accum_positions,1)+1)*nEles
  pos_c=c_loc(accum_positions)

!  call write_gmsh_file('test.msh', positions=accum_positions)
  call deallocate(accum_mesh)
  call deallocate(supermesh_shape)
  call deallocate(supermesh_quad)
  call deallocate(positionsA)
  call deallocate(positionsB)

end subroutine supermesh

subroutine supermesh_sp(X1,N_X1,C1,N_C1,&
                     X2,N_X2,C2,N_C2,&
                     nNodes,nEles,nEntries,pos_c) bind(C)
  use vtkFortranSupermesh
  use iso_c_binding
  use write_gmsh

  implicit none

  integer(kind=c_int), value :: N_X1, N_C1, N_X2, N_C2
  real(kind=c_float) :: X1(3,N_X1), X2(3,N_X2)
  integer (kind=c_long_long) :: C1(N_C1),  C2(N_C2)
  integer(kind=c_int) :: nNodes,nEles,nEntries
  type(c_ptr) :: pos_c

  type(vector_field) :: positionsA, positionsB
  type(ilist), dimension(:), allocatable :: map_BA
  real, dimension(:), allocatable :: tri_detwei
  integer :: ele_A, ele_B
  type(inode), pointer :: llnode
  type(vector_field) :: intersection
  type(element_type) :: supermesh_shape
  type(quadrature_type) :: supermesh_quad
  integer :: dim

  type(mesh_type) :: accum_mesh
  type(vector_field) ::  accum_positions_tmp
  type(vector_field), pointer ::accum_positions

  positionsA=position_field_from_vtk(real(X1),C1,VTK_TRIANGLE)
  positionsB=position_field_from_vtk(real(X2),C2,VTK_TRIANGLE)

  dim = positionsA%dim

  allocate(map_BA(ele_count(positionsB)))

  supermesh_quad = make_quadrature(vertices=dim+1, dim=dim, degree=5)
  supermesh_shape = make_element_shape(vertices=dim+1, dim=dim, degree=1, quad=supermesh_quad)
  allocate(tri_detwei(supermesh_shape%ngi))

  allocate(accum_positions)

  call allocate(accum_mesh, 0, 0, supermesh_shape, "AccumulatedMesh")
  call allocate(accum_positions, dim, accum_mesh, "AccumulatedPositions")

  map_BA = rtree_intersection_finder(positionsB, positionsA)
  call intersector_set_dimension(dim)
  call intersector_set_exactness(.false.)

  ! inputs: positionsB, map_BA, positionsA, supermesh_shape
  ! output: the supermesh!
  call recursive_supermesh(positionsA, positionsB, map_BA, supermesh_shape, 1, ele_count(positionsB), accum_positions)

  nNodes=node_count(accum_positions)
  nEles=element_count(accum_positions)
  nEntries=(ele_loc(accum_positions,1)+1)*nEles
  pos_c=c_loc(accum_positions)

!  call write_gmsh_file('test.msh', positions=accum_positions)
  call deallocate(accum_mesh)
  call deallocate(supermesh_shape)
  call deallocate(supermesh_quad)
  call deallocate(positionsA)
  call deallocate(positionsB)

end subroutine supermesh_sp

subroutine copy_mesh_to_vtk(pos_c, X, nNodes,C ,nEntries) bind(c)
  use vtkFortranSupermesh
  use iso_c_binding
  integer(kind=c_int), intent(in), value :: nNodes,nEntries
  type(c_ptr) :: pos_c
  real(kind=c_double), intent(inout) :: X(3,nNodes)
  integer (kind=c_long_long), intent(inout) :: C(nEntries)

  type(vector_field), pointer :: positions
  integer :: ele, loc

  call c_f_pointer(pos_c,positions)

  X(1:positions%dim,:)=positions%val
  X(positions%dim+1:3,:)=0

  loc=ele_loc(positions,1)
  do ele=1, element_count(positions)
     C((ele-1)*(loc+1)+1)=loc
     C((ele-1)*(loc+1)+2:ele*(loc+1))=ele_nodes(positions,ele)-1
  end do

  call deallocate(positions)
  deallocate(positions)

end subroutine copy_mesh_to_vtk

subroutine KSPDestroy(ksp, ierr)
KSP, pointer :: ksp
integer      :: ierr
end subroutine KSPDestroy
