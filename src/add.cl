__kernel void add(__global int* in1, __global int* in2, __global int* out)
{
    int index = get_global_id(0);
    out[index] = in1[index] + in2[index] ;
}
