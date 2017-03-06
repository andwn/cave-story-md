/**
 *  \file maths.h
 *  \brief Mathematical methods.
 *  \author Stephane Dallongeville
 *  \date 08/2011
 *
 * This unit provides basic maths methods.<br>
 * You can find a tutorial about how use maths with SGDK <a href="http://code.google.com/p/sgdk/wiki/SGDK_Math">here</a>.<br>
 */

#ifndef _MATHS_H_
#define _MATHS_H_

/**
 *  \brief
 *      Returns the lowest value between X an Y.
 */
#define min(X, Y)   (((X) < (Y))?(X):(Y))

/**
 *  \brief
 *      Returns the highest value between X an Y.
 */
#define max(X, Y)   (((X) > (Y))?(X):(Y))

/**
 *  \brief
 *      Returns the absolute value of X.
 */
#define abs(X)      (((X) < 0)?-(X):(X))

#endif // _MATHS_H_
