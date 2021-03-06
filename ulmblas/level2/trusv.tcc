#ifndef ULMBLAS_LEVEL2_TRUSV_TCC
#define ULMBLAS_LEVEL2_TRUSV_TCC 1

#include <ulmblas/level1extensions/axpyf.h>
#include <ulmblas/level1extensions/dotxf.h>
#include <ulmblas/level2/gemv.h>
#include <ulmblas/level2/trusv.h>

namespace ulmBLAS {

template <typename IndexType, typename TA, typename TX>
void
trusv_unblk(IndexType    n,
            bool         unitDiag,
            bool         conjA,
            const TA     *A,
            IndexType    incRowA,
            IndexType    incColA,
            TX           *x,
            IndexType    incX)
{
    for (IndexType i=n-1; i>=0; --i) {
        for (IndexType j=i+1; j<n; ++j) {
            x[i*incX] -= conjugate(A[i*incRowA+j*incColA], conjA)*x[j*incX];
        }
        x[i*incX] = (!unitDiag)
                  ? x[i*incX] / conjugate(A[i*incRowA+i*incColA], conjA)
                  : x[i*incX];
    }
}

template <typename IndexType, typename TA, typename TX>
void
trusv(IndexType    n,
      bool         unitDiag,
      bool         conjA,
      const TA     *A,
      IndexType    incRowA,
      IndexType    incColA,
      TX           *x,
      IndexType    incX)
{
    typedef decltype(TA(0)*TX(0))  T;

    const IndexType    UnitStride(1);

    if (incRowA==UnitStride) {
        const IndexType bf = FuseFactor<T>::axpyf;
        const IndexType nl = n % bf;

        for (IndexType j=n-bf; j>=0; j-=bf) {
            trusv_unblk(bf, unitDiag, conjA,
                        &A[j*UnitStride+j*incColA], UnitStride, incColA,
                        &x[j*incX], incX);

            gemv(j, bf,
                 T(-1), conjA,
                 &A[0*UnitStride+j*incColA], UnitStride, incColA,
                 &x[j*incX], incX,
                 T(1),
                 &x[0*incX], incX);
        }

        trusv_unblk(nl, unitDiag, conjA,
                    &A[0*UnitStride+0*incColA], UnitStride, incColA,
                    &x[0*incX], incX);

    } else if (incColA==UnitStride) {
        const IndexType bf = FuseFactor<T>::dotuxf;
        const IndexType nl = n % bf;

        for (IndexType j=n-bf; j>=0; j-=bf) {
            gemv(bf, n-j-bf,
                 T(-1), conjA,
                 &A[j*incRowA+(j+bf)*UnitStride], incRowA, UnitStride,
                 &x[(j+bf)*incX], incX,
                 T(1),
                 &x[j*incX], incX);

            trusv_unblk(bf, unitDiag, conjA,
                        &A[j*incRowA+j*UnitStride], incRowA, UnitStride,
                        &x[j*incX], incX);
        }

        if (nl) {
            gemv(nl, n-nl,
                 T(-1), conjA,
                 &A[0*incRowA+nl*UnitStride], incRowA, UnitStride,
                 &x[nl*incX], incX,
                 T(1),
                 &x[0*incX], incX);

            trusv_unblk(nl, unitDiag, conjA,
                        &A[0*incRowA+0*UnitStride], incRowA, UnitStride,
                        &x[0*incX], incX);
        }

    } else {
        // TODO: Consider blocking
        trusv_unblk(n, unitDiag, conjA, A, incRowA, incColA, x, incX);
    }
}

template <typename IndexType, typename TA, typename TX>
void
trusv(IndexType    n,
      bool         unitDiag,
      const TA     *A,
      IndexType    incRowA,
      IndexType    incColA,
      TX           *x,
      IndexType    incX)
{
    trusv(n, unitDiag, false, A, incRowA, incColA, x, incX);
}

} // namespace ulmBLAS

#endif // ULMBLAS_LEVEL2_TRUSV_TCC
