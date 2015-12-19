#include "fdebug.h"

module vtkFortranSupermesh


  use elements
  use shape_Functions
  use fields
  use iso_c_binding
  use unify_meshes_module




  implicit none

  type supermesh_data
     integer(kind=c_int), dimension(:), allocatable :: ele_map_A, ele_map_B
     type(vector_field) :: positions
  end type supermesh_data

  integer, parameter :: VTK_VERTEX=1, VTK_LINE=3, VTK_TRIANGLE=5,&
       VTK_TETRA=10

#include "supermesh_femtools/petsc_legacy.h"

contains

  function vertex_measure(X) result(measure)
    
    real, dimension(:,:) :: X

    real :: measure

    select case(size(X,2))
       case(1)
          measure=1.0
       case(2)
          measure=sqrt(sum((X(:,2)-X(:,1))**2))
       case(3)
          measure=abs(cross2( X(:,2)-X(:,1), X(:,3)-X(:,1)))/2.0
       case(4)
          measure=dot_product(X(:,2)-X(:,1),cross3(X(:,3)-X(:,1),X(:,4)-X(:,1)))/6.0
       end select

       contains
         
         function cross2(a,b)
           real, dimension(2) :: a,b
           real :: cross2

           cross2 = a(1)*b(2)-a(2)*b(1)

         end function cross2

         function cross3(a,b)
           real, dimension(3) :: a,b
           real, dimension(3) :: cross3

           cross3(1)=a(2)*b(3)-a(3)*b(2)
           cross3(2)=a(3)*b(1)-a(1)*b(3)
           cross3(3)=a(1)*b(2)-a(2)*b(1)
         end function cross3
       end function vertex_measure

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

  subroutine recursive_supermesh(positionsA, positionsB, map_BA, supermesh_shape, start_ele, end_ele, ele_map_A, ele_map_B, supermesh)
    type(vector_field), intent(in) :: positionsA, positionsB
    type(ilist), dimension(:), intent(in) :: map_BA
    type(element_type), intent(in) :: supermesh_shape
    integer, intent(in) :: start_ele, end_ele
    type(vector_field), intent(out) :: supermesh

    integer, parameter :: blocksize = 4
    integer :: i, J

    type(vector_field) :: supermesh_tmp, supermesh_full
    type(mesh_type) :: supermesh_mesh
    type(vector_field) :: supermesh_accum
    type(inode), pointer :: llnode

    integer :: ele_A, ele_B, ele_C, ele_tmp, ele_size
    integer :: new_start, new_end, step

    integer :: nintersections, nintersections_max
    integer, dimension(:), allocatable :: ele_map_A, ele_map_B
    real, dimension(:), allocatable    :: mass
    integer, dimension(:, :), allocatable :: intersection_parents
    type(vector_field), dimension(:), allocatable :: intersections

!    if (all(map_BA%length==1)) then
!       supermesh=positionsA
!       incref(positionsA)
!    allocate(ele_map_A(ele_size))
!    allocate(ele_map_B(ele_size))
!    ele_map_A=
!    return
!    end if
      
       

    nintersections_max = sum(map_BA%length)
    ewrite(2, "(a,i0)") "Maximum number of intersections: ", nintersections_max
    allocate(intersections(nintersections_max))
    allocate(intersection_parents(nintersections_max, 2))
    nintersections = 0


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


!    call allocate(supermesh_mesh, 0, 0, supermesh_shape, "AccumulatedMesh")
!    call allocate(supermesh_tmp, positionsA%dim, supermesh_mesh, "AccumulatedPositions")    
    supermesh_full = unify_meshes(intersections(:nintersections))
    allocate(mass(ele_count(supermesh_full)))

    do i =1, ele_count(supermesh_full)
       mass(i)=vertex_measure(ele_val(supermesh_full,i))
    end do

    ele_size=count(mass>0.0)

    call allocate(supermesh_mesh, supermesh_shape%loc*ele_size, ele_size, supermesh_shape, "Supermesh")
    do i =1, ele_size
       call set_ele_nodes(supermesh_mesh, i, &
            [((i-1)*supermesh_shape%loc + j,j=1,supermesh_shape%loc)])
    end do

    call allocate(supermesh, positionsA%dim, supermesh_mesh, "Coordinate")
    ele_C=0
    do i =1, ele_count(supermesh_full)
       if (mass(i)==0.0) cycle
       ele_C = ele_C + 1
       call set(supermesh, ele_nodes(supermesh,ele_C), ele_val(supermesh_full,i))
    end do

    assert(ele_C == ele_size)

    call deallocate(supermesh_full)
    call deallocate(supermesh_mesh)
    
    allocate(ele_map_A(ele_size))
    allocate(ele_map_B(ele_size))

    ele_tmp=0
    ele_C=0
    do i = 1, nintersections
       do j = 1, ele_count(intersections(i))
          ele_tmp = ele_tmp + 1
          if (mass(ele_tmp)==0.0) cycle
          ele_C = ele_C + 1
          ele_map_A(ele_C) = intersection_parents(i, 1)
          ele_map_B(ele_C) = intersection_parents(i, 2)
       end do
       call deallocate(intersections(i))
    end do
  
  end subroutine recursive_supermesh

end module vtkFortranSupermesh

function position_from_vtk_c(X,N_X,C,N_C) result(ptr) bind(c)
  use vtkFortranSupermesh
  use iso_c_binding
  integer(kind=c_int), value :: N_X, N_C
  real(kind=c_double) :: X(3,N_X)
  integer (kind=c_long_long) :: C(N_C)  

  type(c_ptr) :: ptr

  type(vector_field), pointer :: positions

  allocate(positions)

  select case(C(1))
  case(1)
     positions=position_field_from_vtk(X,C,VTK_VERTEX)
  case(2)
     positions=position_field_from_vtk(X,C,VTK_LINE)
  case(3)
     positions=position_field_from_vtk(X,C,VTK_TRIANGLE)
  case(4)
     positions=position_field_from_vtk(X,C,VTK_TETRA)
  end select

  ptr=c_loc(positions)

end function position_from_vtk_c

subroutine position_from_vtk_c_sp(X, N_X, C, N_C, ptr) bind(c)
  use vtkFortranSupermesh
  use iso_c_binding
  integer(kind=c_int), value :: N_X, N_C
  real(kind=c_float) :: X(3,N_X)
  integer (kind=c_long_long) :: C(N_C)  
  type(c_ptr) :: ptr

  type(vector_field), pointer :: positions

  allocate(positions)

  select case(C(1))
  case(1)
     positions=position_field_from_vtk(real(X),C,VTK_VERTEX)
  case(2)
     positions=position_field_from_vtk(real(X),C,VTK_LINE)
  case(3)
     positions=position_field_from_vtk(real(X),C,VTK_TRIANGLE)
  case(4)
     positions=position_field_from_vtk(real(X),C,VTK_TETRA)
  end select

  ptr=c_loc(positions)

end subroutine position_from_vtk_c_sp
  
subroutine supermesh(ptr1, ptr2, nNodes, nEles,nEntries, context_c) bind(C)
  use vtkFortranSupermesh
  use iso_c_binding
  use write_gmsh

  implicit none

  type(c_ptr), intent(in), value :: ptr1, ptr2
  integer(kind=c_int) :: nNodes,nEles,nEntries
  type(c_ptr) :: context_c

  type(vector_field), pointer :: positionsA, positionsB
  type(ilist), dimension(:), allocatable :: map_BA
  real, dimension(:), allocatable :: tri_detwei
  integer :: ele_A, ele_B
  type(inode), pointer :: llnode
  type(vector_field) :: intersection
  type(element_type) :: supermesh_shape
  type(quadrature_type) :: supermesh_quad
  integer :: dim

  type(mesh_type) :: accum_mesh

  type(supermesh_data), pointer :: data
  type(vector_field) ::  accum_positions_tmp

  call c_f_pointer(ptr1,positionsA)
  call c_f_pointer(ptr2,positionsB)

  dim = positionsA%dim

  allocate(map_BA(ele_count(positionsB)))

  supermesh_quad = make_quadrature(vertices=dim+1, dim=dim, degree=5)
  supermesh_shape = make_element_shape(vertices=dim+1, dim=dim, degree=1, quad=supermesh_quad)
  allocate(tri_detwei(supermesh_shape%ngi))

  allocate(data)

  call allocate(accum_mesh, 0, 0, supermesh_shape, "AccumulatedMesh")
  call allocate(data%positions, dim, accum_mesh, "AccumulatedPositions")

  map_BA = rtree_intersection_finder(positionsB, positionsA)
  call intersector_set_dimension(dim)

  ! inputs: positionsB, map_BA, positionsA, supermesh_shape
  ! output: the supermesh!
  call recursive_supermesh(positionsA, positionsB, map_BA, supermesh_shape, 1, ele_count(positionsB), data%ele_map_A, data%ele_map_B,data%positions)

  nNodes=node_count(data%positions)
  nEles=element_count(data%positions)
  nEntries=(ele_loc(data%positions,1)+1)*nEles
  context_c=c_loc(data)

  call deallocate(accum_mesh)
  call deallocate(supermesh_shape)
  call deallocate(supermesh_quad)
  call deallocate(positionsA)
  call deallocate(positionsB)
  deallocate(positionsA)
  deallocate(positionsB)

end subroutine supermesh

subroutine copy_mesh_to_vtk(context_c, X, nNodes,C ,nEntries,&
     ele_map_0, ele_map_1, nEles) bind(c)
  use vtkFortranSupermesh
  use iso_c_binding
  integer(kind=c_int), intent(in), value :: nNodes, nEntries, nEles
  type(c_ptr), value :: context_c
  real(kind=c_double), intent(inout) :: X(3,nNodes)
  integer (kind=c_long_long), intent(inout) :: C(nEntries)
  integer(kind=c_int), intent(inout), dimension(nEles) :: ele_map_0, ele_map_1

  type(supermesh_data), pointer :: data
  integer :: ele, loc

  call c_f_pointer(context_c, data)

  X(1:data%positions%dim,:)=data%positions%val
  X(data%positions%dim+1:3,:)=0

  loc=ele_loc(data%positions,1)
  do ele=1, element_count(data%positions)
     C((ele-1)*(loc+1)+1)=loc
     C((ele-1)*(loc+1)+2:ele*(loc+1))=ele_nodes(data%positions,ele)-1
  end do
  
  ele_map_0=data%ele_map_A-1
  ele_map_1=data%ele_map_B-1

  call deallocate(data%positions)
  deallocate(data%ele_map_A,data%ele_map_B)
  deallocate(data)

end subroutine copy_mesh_to_vtk

subroutine KSPDestroy(ksp, ierr)
KSP, pointer :: ksp
integer      :: ierr
end subroutine KSPDestroy
