/*****************************************************************************
 *
 * file:   sac_errors.h
 *
 * description:
 *   This file is part of the SAC standard header file sac.h.
 *   It provides macros for runtime errors.
 *
 *****************************************************************************/

#ifndef _SAC_ERRORS_H_
#define _SAC_ERRORS_H_

/******************************************************************************
 *
 * ICM for type errors that are behind conditionals:
 * =================================================
 *
 * TYPE_ERROR( error_message)
 *
 ******************************************************************************/

#define SAC_TYPE_ERROR(message) SAC_RuntimeError (message);

#endif /* _SAC_ERRORS_H_ */
