#ifndef MATRIX_H

#define MATRIX_H

class B_Vector;

class B_Matrix
{
public:
    B_Matrix();
    B_Matrix(const B_Vector &v);
    B_Vector TranslationVector() const;
    void SetTranslation(const B_Vector &v);
    void RigidInverse(B_Matrix &out) const;
private:
    double m[4][4];
    friend void B_MultiplyMatrices(B_Matrix &, const B_Matrix &, const B_Matrix &);
    friend B_Vector Rotate(const B_Vector &, const B_Matrix &);
};

extern B_Vector operator *(const B_Vector& vector, const B_Matrix &matrix);
extern B_Vector Rotate(const B_Vector &v, const B_Matrix &rotateMatrix);
extern void B_MultiplyMatrices(B_Matrix &out, const B_Matrix &a, const B_Matrix &b);

#endif
