/* -*- mode: C++; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
// vim:sts=4:sw=4:ts=4:noet:sr:cino=>s,f0,{0,g0,(0,\:0,t0,+0,=s
/*
 * Copyright (C) 2014 FFLAS-FFACK group
 *
 * Written by Clement Pernet <Clement.Pernet@imag.fr>
 * Brice Boyer (briceboyer) <boyer.brice@gmail.com>
 *
 *
 * ========LICENCE========
 * This file is part of the library FFLAS-FFPACK.
 *
 * FFLAS-FFPACK is free software: you can redistribute it and/or modify
 * it under the terms of the  GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * ========LICENCE========
 *.
 */

#ifndef __FFLASFFPACK_ffpack_ftrtr_INL
#define __FFLASFFPACK_ffpack_ftrtr_INL


namespace FFPACK {


	template<class Field>
	void
	ftrtri (const Field& F, const FFLAS::FFLAS_UPLO Uplo, const FFLAS::FFLAS_DIAG Diag,
			const size_t N, typename Field::Element_ptr A, const size_t lda)
	{
		std::cout << "rec call" << std::endl;
		if (!N) return;
		if (N <= 5){ // base case
			if (Uplo == FFLAS::FflasUpper){
				for(size_t li = N-1; li < N; li--){ // since li is size_t when it goes below 0 it will overflow above size
					if(Diag == FFLAS::FflasNonUnit)
						F.invin(A[(lda+1)*li]);            //invert element on diagonal
					for(size_t co = N-1; co > li; co--){
						fgemv(F,FFLAS::FflasNoTrans,1,co-li,
							  A[li*(lda+1)],               // diagonal element of the current line
							  (A+li*(lda+1)+1),1,          // horizontal vector starting after the diagonal left of the current element
							  (A+co+lda*(li+1)),lda,       // vertical vector starting below the current element
							  F.zero,(A+co+lda*li),lda);   // The currrent element
						F.negin(A[co+lda*li]);
					}
				}
			}
			else{ // Uplo == FflasLower
				for(size_t li = 0; li < N; li++){
					F.invin(A[(lda+1)*li]);         //invert element on diagonal
					for(size_t co = 0; co < li; co++){
						fgemv(F,FFLAS::FflasNoTrans,1,li-co,
							  *(A+li*(lda+1)),        // diagonal element of the current line
							  (A+co+li*lda),1,        // horizontal vector starting at the current element
							  (A+co*(lda+1)),lda,     // vertical vector starting at the diagonal element above the current element
							  F.zero,
							  (A+co+lda*li),lda);     // The currrent element
						F.negin(A[co+lda*li]);
					}
				}
			}
		}
		else { // recursive case
			size_t N1 = N/2;
			size_t N2 = N - N1;
			ftrtri (F, Uplo, Diag, N1, A, lda);
			ftrtri (F, Uplo, Diag, N2, A + N1*(lda+1), lda);
			if (Uplo == FFLAS::FflasUpper){
				ftrmm (F, FFLAS::FflasLeft, Uplo, FFLAS::FflasNoTrans, Diag, N1, N2,
					   F.one, A, lda, A + N1, lda);
				ftrmm (F, FFLAS::FflasRight, Uplo, FFLAS::FflasNoTrans, Diag, N1, N2,
					   F.mOne, A + N1*(lda+1), lda, A + N1, lda);
			}
			else {
				ftrmm (F, FFLAS::FflasLeft, Uplo, FFLAS::FflasNoTrans, Diag, N2, N1,
					   F.one, A + N1*(lda+1), lda, A + N1*lda, lda);
				ftrmm (F, FFLAS::FflasRight, Uplo, FFLAS::FflasNoTrans, Diag, N2, N1,
					   F.mOne, A, lda, A + N1*lda, lda);
			}
		}
	}
	
	template<class Field>
	void
	ftrtrm (const Field& F, const FFLAS::FFLAS_SIDE side, const FFLAS::FFLAS_DIAG diag,
			const size_t N,	typename Field::Element_ptr A, const size_t lda)
	{
		if (N <= 1)
			return;
		size_t N1 = N/2;
		size_t N2 = N-N1;

		if (side == FFLAS::FflasLeft){
			ftrtrm (F, side, diag, N1, A, lda);
			
			FFLAS::fgemm (F, FFLAS::FflasNoTrans, FFLAS::FflasNoTrans, N1, N1, N2, F.one,
			       A+N1, lda, A+N1*lda, lda, F.one, A, lda);
			
			FFLAS::ftrmm (F, FFLAS::FflasRight, FFLAS::FflasLower, FFLAS::FflasNoTrans,
			       (diag == FFLAS::FflasUnit) ? FFLAS::FflasNonUnit : FFLAS::FflasUnit,
			       N1, N2, F.one, A + N1*(lda+1), lda, A + N1, lda);
			
			FFLAS::ftrmm (F, FFLAS::FflasLeft, FFLAS::FflasUpper, FFLAS::FflasNoTrans, diag, N2, N1,
			       F.one, A + N1*(lda+1), lda, A + N1*lda, lda);
			
			ftrtrm (F, side, diag, N2, A + N1*(lda+1), lda);
		} else { // side = FflasRight
			ftrtrm (F, side, diag, N2, A + N1*(lda+1), lda);

			FFLAS::fgemm (F, FFLAS::FflasNoTrans, FFLAS::FflasNoTrans, N2, N2, N1, F.one,
			       A+N1*lda, lda, A+N1, lda, F.one, A + N1*(lda+1), lda);
			
			FFLAS::ftrmm (F, FFLAS::FflasLeft, FFLAS::FflasLower, FFLAS::FflasNoTrans,
			       (diag == FFLAS::FflasUnit) ? FFLAS::FflasNonUnit : FFLAS::FflasUnit,
			       N1, N2, F.one, A, lda, A + N1, lda);
			
			FFLAS::ftrmm (F, FFLAS::FflasRight, FFLAS::FflasUpper, FFLAS::FflasNoTrans, diag, N2, N1,
			       F.one, A, lda, A + N1*lda, lda);

			ftrtrm (F, side, diag, N1, A, lda);
		}
	}

	template<class Field>
	void trinv_left( const Field& F, const size_t N, typename Field::ConstElement_ptr L, const size_t ldl,
			 typename Field::Element_ptr X, const size_t ldx )
	{
		FFLAS::fassign(F,N,N,L,ldl,X,ldx);
		ftrtri (F, FFLAS::FflasLower, FFLAS::FflasUnit, N, X, ldx);
		//invL(F,N,L,ldl,X,ldx);
	}

} // FFPACK

#endif // __FFLASFFPACK_ffpack_ftrtr_INL
