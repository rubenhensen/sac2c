!>
!! Fortran Interface for SAC Runtime System
!! 
!!   This file is included as part of the fwrapper.f file that is
!!   generated through `sac4c -fortran` - it can not be used on it own!
!!
!!   It implicitly makes use of the functions declared in sacinterface.h.
!!
!!   Last updated Oct 2015
!!

!************************************************************************
!> SACARG related functions
!************************************************************************
              integer(c_int) function SACARGgetDim
     &                (sacarg)
     &                bind(c, name='SACARGgetDim')
                  import
                  implicit none
                  type(c_ptr), value, intent(in) :: sacarg
              end function SACARGgetDim
              integer(c_int) function SACARGgetShape
     &            (sacarg, pos)
     &            bind(c, name = 'SACARGgetShape')
                  import
                  implicit none
                  type(c_ptr), value, intent(in) :: sacarg
                  integer(c_int), value, intent(in) :: pos
              end function SACARGgetShape
!************************************************************************
!> SAC Runtime related functions
!************************************************************************
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
!************************************************************************
!> SAC Thread-managment related functions
!************************************************************************
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
!************************************************************************
!> SACARG Conversion functions 
!************************************************************************
              type(c_ptr) function SACARGconvertFromIntScalar
     &            (num)
     &            bind(c, name = "SACARGconvertFromIntScalar")
                  import
                  implicit none
                  integer(c_int), value, intent(in) :: num
              end function SACARGconvertFromIntScalar
              type(c_ptr) function SACARGconvertFromIntPointerVect
     &            (matrix, dims, shape)
     &            bind(c, name = 'SACARGconvertFromIntPointerVect')
                  import
                  implicit none
                  type(c_ptr), value, intent(in) :: matrix
                  type(c_ptr), value, intent(in) :: shape
                  integer(c_int), value, intent(in) :: dims
              end function SACARGconvertFromIntPointerVect
              type(c_ptr) function SACARGconvertFromDoublePointerVect
     &            (matrix, dims, shape)
     &            bind(c, name = 'SACARGconvertFromDoublePointerVect')
                  import
                  implicit none
                  type(c_ptr), value, intent(in) :: matrix
                  type(c_ptr), value, intent(in) :: shape
                  integer(c_int), value, intent(in) :: dims
              end function SACARGconvertFromDoublePointerVect
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
