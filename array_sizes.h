/******************************************************************************
 *
 * array_sizes.h, Version 1.4.3 Sat 09 Jul 2011 
 *
 * ----------------------------------------------------------------------------
 *
 * Epp (Easy particle propagation)
 * Copyright (C) 2011 CancerCare Manitoba
 *
 * The latest version of Epp and additional information are available online at 
 * http://www.physics.umanitoba.ca/~elbakri/epp/
 *
 * Epp is based on the EGSnrc C++ class library which is available online at
 * http://www.irs.inms.nrc.ca/EGSnrc/pirs898/
 *
 * Epp is free software; you can redistribute it and/or modify it under the 
 * terms of the GNU General Public License as published by the Free Software 
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.                                       
 *                                                                           
 * Epp is distributed in the hope that it will be useful, but WITHOUT ANY 
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more 
 * details.                              
 *                                                                           
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * ----------------------------------------------------------------------------
 *
 *   Contact:
 *
 *   Jonas Lippuner
 *   CancerCare Manitoba, Department of Medical Physics
 *   675 McDermot Ave, Winnipeg, Manitoba, R3E 0V9, Canada
 *   Email: jonas@lippuner.ca 
 *
 *****************************************************************************/

/******************************************************************************
 *
 *   Define in this file the maximum number of media (MXMED) and the
 *   maximum number of particles on the stack (MXSTACK).
 *   
 *   This file gets included by the egsnrc fortran subroutines 
 *   (egsnrc_$my_machine.F), the base application 
 *   (egs_advanced_xml_application.cpp), and possible by the user code, if use 
 *   is made of the particle stack or one of the structures that depends on the
 *   maximum number of media.
 *
 *****************************************************************************/

#ifndef ARRAY_SIZES_
#define ARRAY_SIZES_
#define MXMED 50
#define MXSTACK 200000
#endif
