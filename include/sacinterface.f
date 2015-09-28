!
! Fortran Interface for SAC Runtime System
!
              subroutine SAC_InitRuntimeSystem
     &                ()
     &                bind(c, name = 'SAC_InitRuntimeSystem')
                  import
                  implicit none
              end subroutine SAC_InitRuntimeSystem
              subroutine SAC_FreeRuntimeSystem
     &                ()
     &                bind(c, name = 'SAC_FreeRuntimeSystem')
                  import
                  implicit none
              end subroutine SAC_FreeRuntimeSystem
              type(c_ptr) function SAC_AllocHive
     &                (threads, scheduler, places, bdata)
     &                bind(c, name = 'SAC_AllocHive')
                  import
                  implicit none
                  integer(c_int), value, intent(in) :: threads
                  integer(c_int), value, intent(in) :: scheduler
                  type(c_ptr), value, intent(in) :: places
                  type(c_ptr), value, intent(in) :: bdata
              end function SAC_AllocHive
              subroutine SAC_AttachHive
     &                (hive)
     &                bind(c, name = 'SAC_AttachHive')
                  import
                  implicit none
                  type(c_ptr), value, intent(in) :: hive
              end subroutine SAC_AttachHive
              subroutine SAC_ReleaseQueen
     &                ()
     &                bind(c, name = 'SAC_ReleaseQueen')
                  import
                  implicit none
              end subroutine SAC_ReleaseQueen
              type(c_ptr) function SAC_DetachHive
     &                ()
     &                bind(c, name = 'SAC_DetachHive')
                  import
                  implicit none
              end function SAC_DetachHive
              type(c_ptr) function SACARGconvertFromIntScalar
     &            (num)
     &            bind(c, name = "SACARGconvertFromIntScalar")
                  import
                  implicit none
                  integer(c_int), value, intent(in) :: num
              end function SACARGconvertFromIntScalar
              type(c_ptr) function SACARGconvertFromIntPointer2
     &            (matrix, dims, rows, columns)
     &            bind(c, name = 'SACARGconvertFromIntPointer')
                  import
                  implicit none
                  type(c_ptr), value, intent(in) :: matrix
                  integer(kind = c_int), value, intent(in) :: dims
                  integer(kind = c_int), value, intent(in) :: rows
                  integer(kind = c_int), value, intent(in) :: columns
              end function SACARGconvertFromIntPointer2
              type(c_ptr) function SACARGconvertFromDoublePointer1
     &            (array, dim, elements)
     &            bind(c, name = "SACARGconvertFromDoublePointer")
                  import
                  implicit none
                  type(c_ptr), value, intent(in) :: array
                  integer(c_int), value, intent(in) :: dim
                  integer(c_int), value, intent(in) :: elements
              end function SACARGconvertFromDoublePointer1
              type(c_ptr) function SACARGconvertFromDoublePointer2
     &            (array, dim, element0, element1)
     &            bind(c, name = "SACARGconvertFromDoublePointer")
                  import
                  implicit none
                  type(c_ptr), value, intent(in) :: array
                  integer(c_int), value, intent(in) :: dim
                  integer(c_int), value, intent(in) :: element0
                  integer(c_int), value, intent(in) :: element1
              end function SACARGconvertFromDoublePointer2
              type(c_ptr) function SACARGconvertToDoubleArray
     &            (result)
     &            bind(c, name = "SACARGconvertToDoubleArray")
                  import
                  implicit none
                  type(c_ptr), value, intent(in) :: result
              end function SACARGconvertToDoubleArray
              type(c_ptr) function SACARGconvertToIntArray
     &            (result)
     &            bind(c, name = "SACARGconvertToIntArray")
                  import
                  implicit none
                  type(c_ptr), value, intent(in) :: result
              end function SACARGconvertToIntArray
              integer(kind = c_int) function SACARGgetShape
     &            (arg, pos)
     &            bind(c, name = 'SACARGgetShape')
                  import
                  implicit none
                  type(c_ptr), value, intent(in) :: arg
                  integer(kind = c_int), value, intent(in) :: pos
              end function SACARGgetShape
