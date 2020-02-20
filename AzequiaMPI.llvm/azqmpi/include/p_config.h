/* _________________________________________________________________________
   |                                                                       |
   |  Azequia (embedded) Message Passing Interface   ( AzequiaMPI )        |
   |                                                                       |
   |  Authors: DSP Systems Group                                           |
   |           http://gsd.unex.es                                          |
   |           University of Extremadura                                   |
   |           Caceres, Spain                                              |
   |           jarico@unex.es                                              |
   |                                                                       |
   |  Date:    Sept 22, 2008                                               |
   |                                                                       |
   |  Description:                                                         |
   |                                                                       |
   |                                                                       |
   |_______________________________________________________________________| */

#ifndef	_CONFIG_H
#define	_CONFIG_H


  /*----------------------------------------------------------------/
 /   Declaration of types and functions used by this module        /
/----------------------------------------------------------------*/
#include <config.h>

#include <thr.h>
#include <addr.h>
#include <com.h>

  /*-------------------------------------------------------/
 /                   Public constants                     /
/-------------------------------------------------------*/
/* Versions supported */
#define AZQMPI_VERSION	 "1.5"
#define AZQMPI_REVISION  "5"

/* Private functions */
#define PRIVATE static
#define PUBLIC

/* Max integer number */
#define MAX_INTEGER		0x7FFFFFFF

#endif
