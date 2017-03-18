__kernel void adds(__global int* in1,__global int* in2,__global int* in3, __global int* out)
{
    int index = get_global_id(0);
    out[index] = in1[index]+in2[index]+in3[index];
}
