#include "ext.h"			// standard Max include, always required (except in Jitter)
#include "ext_obex.h"		// required for "new" style objects
#include "z_dsp.h"			// required for MSP objects
#include<simd/simd.h>

typedef struct {
	t_pxobject const super;
    double const coefs[5];
    double const eps;
    struct {
        double x[2];
        double y[2];
    } series;
} t_duffing;

C74_HIDDEN t_class const * class = NULL;

C74_HIDDEN t_duffing const*const duffing_new(void) {
    register t_duffing*this = NULL;
    if ((this = (t_duffing*const)object_alloc((t_class*const)class))) {
        z_dsp_setup((t_pxobject*const)this, 7);
        outlet_new(this, "signal");
        *(double*const)(this->coefs + 0) = 1;
        *(double*const)(this->coefs + 1) = 0;
        *(double*const)(this->coefs + 2) = 0;
        *(double*const)(this->coefs + 3) = 0;
        *(double*const)(this->coefs + 4) = 0;
        *(double*const)&this->eps = 0;
    }
    return this;
}

C74_HIDDEN void duffing_synth(double*const y,
                              double const*const x,
                              double const*const b0, long const b0s,
                              double const*const b1, long const b1s,
                              double const*const b2, long const b2s,
                              double const*const a1, long const a1s,
                              double const*const a2, long const a2s,
                              double const*const e0, long const e0s,
                              double*const xs,
                              double*const ys,
                              long const length) {
    register double x0 = xs[0], x1 = xs[1], x2 = 0;
    register double y0 = ys[0], y1 = ys[1], y2 = 0;
    for ( register long t = 0, T = length ; t < T ; ++ t ) {
        register simd_double3 const b = {b0[t*b0s], b1[t*b1s], b2[t*b2s]};
        register simd_double3 const a = {       -1, a1[t*a1s], a2[t*a2s]};
        register double const eps = e0[t*e0s];
        register double const epsp1 = eps + 1;
        x2 = x1;
        x1 = x0;
        x0 = x[t];
        y2 = y1;
        y1 = y0;
        register double const v = simd_dot((simd_double3 const) { simd_dot((simd_double3 const) {x0, x1, x2}, b), y1, y2}, a);
        register double u = -v, eu2;
        eu2 = eps * u * u; u -= fma(epsp1, v, fma(eu2, u, u)) / fma(3, eu2, 1);
        eu2 = eps * u * u; u -= fma(epsp1, v, fma(eu2, u, u)) / fma(3, eu2, 1);
        eu2 = eps * u * u; u -= fma(epsp1, v, fma(eu2, u, u)) / fma(3, eu2, 1);
        y[t] = y0 = u;
    }
    xs[0] = x0;
    xs[1] = x1;
    ys[0] = y0;
    ys[1] = y1;
}

C74_HIDDEN void duffing_entry(t_duffing const*const this, t_object const*const dsp64, double const*const*const i, long const ic, double*const*const o, long const oc, long const length, long const flags, void*const parameter) {
    register uintptr_t const connections = (uintptr_t const)parameter;
    duffing_synth(*o,
                  *i,
                  connections >> 5 & 1 ? i[1] : this->coefs + 0, connections >> 5 & 1,
                  connections >> 4 & 1 ? i[2] : this->coefs + 1, connections >> 4 & 1,
                  connections >> 3 & 1 ? i[3] : this->coefs + 2, connections >> 3 & 1,
                  connections >> 2 & 1 ? i[4] : this->coefs + 3, connections >> 2 & 1,
                  connections >> 1 & 1 ? i[5] : this->coefs + 4, connections >> 1 & 1,
                  connections & 1 ? i[6] : &this->eps, connections & 1,
                  (double*const)this->series.x,
                  (double*const)this->series.y,
                  length);
}

C74_HIDDEN void duffing_dsp64(t_duffing const*const this, t_object const*const dsp64, unsigned short const*const count, double const samplerate, long const maxvectorsize, long const flags) {
    register uintptr_t const connections = (!!count[1]) << 5 | (!!count[2]) << 4 | (!!count[3]) << 3 | (!!count[4]) << 2 | (!!count[5]) << 1 | (!!count[6]);
    *(double*const)(this->series.x + 0) = 0;
    *(double*const)(this->series.x + 1) = 0;
    *(double*const)(this->series.y + 0) = 0;
    *(double*const)(this->series.y + 1) = 0;
    dsp_add64((t_object*const)dsp64, (t_object*const)this, (t_perfroutine64 const)duffing_entry, 0, (uintptr_t const)connections);
}

C74_HIDDEN void duffing_coefs(t_duffing const*const this, t_symbol const*const symbol, unsigned short const argc, t_atom const*const argv) {
    switch ( argc ) {
        case 5:
            switch ( atom_getdouble_array(argc, (t_atom*const)argv, argc, (double*const)this->coefs) ) {
                case MAX_ERR_NONE:
                    break;
                default:
                    object_error((t_object*const)this, "message should be array of number");
                    break;
            }
            break;
        default:
            object_error((t_object*const)this, "invalid message length");
            break;
    }
}

C74_HIDDEN void duffing_value(t_duffing const*const this, double const value) {
    switch ( proxy_getinlet((t_object*const)this) ) {
        case 1:
            *(double*const)(this->coefs + 0) = value;
            break;
        case 2:
            *(double*const)(this->coefs + 1) = value;
            break;
        case 3:
            *(double*const)(this->coefs + 2) = value;
            break;
        case 4:
            *(double*const)(this->coefs + 3) = value;
            break;
        case 5:
            *(double*const)(this->coefs + 4) = value;
            break;
        case 6:
            *(double*const)(&this->eps) = value;
            break;
        default:
            object_error((t_object*const)this, "invalid inlet");
            break;
    }
}

C74_EXPORT void ext_main(void*const r) {
    if ((class = (t_class*const)class_new("duffing~", (method const)duffing_new, (method const)z_dsp_free, (long const)sizeof(t_duffing), 0L, 0))) {
        class_addmethod((t_class*const)class, (method const)duffing_coefs, "list", A_GIMME, 0);
        class_addmethod((t_class*const)class, (method const)duffing_dsp64, "dsp64", A_CANT, 0);
        class_addmethod((t_class*const)class, (method const)duffing_value, "float", A_FLOAT, 0);
        class_dspinit((t_class*const)class);
        class_register(CLASS_BOX, (t_class*const)class);
    }
}
