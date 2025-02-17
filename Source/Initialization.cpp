#include "MagneX.H"

#include "AMReX_ParmParse.H"
#include "AMReX_Parser.H"

void InitializeMagneticProperties(MultiFab& Ms,
                                  MultiFab& gamma,
                                  MultiFab& exchange,
                                  MultiFab& DMI,
                                  MultiFab& anisotropy,
                                  const Geometry& geom,
                                  const Real& time)
{
    // timer for profiling
    BL_PROFILE_VAR("InitializeMagneticProperties()",InitializeMagneticProperties);

    // Ms is the only material property with ghost cells
    Ms.setVal(0.);
    
    // extract dx from the geometry object
    GpuArray<Real,AMREX_SPACEDIM> dx = geom.CellSizeArray();

    ParmParse pp;

    std::string Ms_parser_string;
    pp.get("Ms_parser(x,y,z)",Ms_parser_string);
    Parser Ms_parser(Ms_parser_string);
    Ms_parser.registerVariables({"x","y","z"});
    auto Ms_p = Ms_parser.compile<3>();

    for (MFIter mfi(Ms); mfi.isValid(); ++mfi)
    {
        const Box& bx = mfi.growntilebox(1); // one ghost cell

        const Array4<Real>& Ms_arr = Ms.array(mfi);

        amrex::ParallelFor( bx, [=] AMREX_GPU_DEVICE (int i, int j, int k)
        {
            Real x = prob_lo[0] + (i+0.5) * dx[0];
            Real y = prob_lo[1] + (j+0.5) * dx[1];
            Real z = prob_lo[2] + (k+0.5) * dx[2];

            Ms_arr(i,j,k) = Ms_p(x,y,z);
        });
    }

    std::string gamma_parser_string;
    pp.get("gamma_parser(x,y,z)",gamma_parser_string);
    Parser gamma_parser(gamma_parser_string);
    gamma_parser.registerVariables({"x","y","z"});
    auto gamma_p = gamma_parser.compile<3>();

    for (MFIter mfi(gamma,TilingIfNotGPU()); mfi.isValid(); ++mfi)
    {
        const Box& bx = mfi.tilebox(); // no ghost cells

        const Array4<Real>& gamma_arr = gamma.array(mfi);

        amrex::ParallelFor( bx, [=] AMREX_GPU_DEVICE (int i, int j, int k)
        {
            Real x = prob_lo[0] + (i+0.5) * dx[0];
            Real y = prob_lo[1] + (j+0.5) * dx[1];
            Real z = prob_lo[2] + (k+0.5) * dx[2];

            gamma_arr(i,j,k) = gamma_p(x,y,z);

        });
    }

    if (exchange_coupling == 1) {
        std::string exchange_parser_string;
        pp.get("exchange_parser(x,y,z)",exchange_parser_string);
        Parser exchange_parser(exchange_parser_string);
        exchange_parser.registerVariables({"x","y","z"});
        auto exchange_p = exchange_parser.compile<3>();

        for (MFIter mfi(exchange,TilingIfNotGPU()); mfi.isValid(); ++mfi)
        {
            const Box& bx = mfi.tilebox(); // no ghost cells

            const Array4<Real>& exchange_arr = exchange.array(mfi);

            amrex::ParallelFor( bx, [=] AMREX_GPU_DEVICE (int i, int j, int k)
            {
                Real x = prob_lo[0] + (i+0.5) * dx[0];
                Real y = prob_lo[1] + (j+0.5) * dx[1];
                Real z = prob_lo[2] + (k+0.5) * dx[2];

                exchange_arr(i,j,k) = exchange_p(x,y,z);

            }); 
        }
    }

    if (DMI_coupling == 1) {
        std::string DMI_parser_string;
        pp.get("DMI_parser(x,y,z)",DMI_parser_string);
        Parser DMI_parser(DMI_parser_string);
        DMI_parser.registerVariables({"x","y","z"});
        auto DMI_p = DMI_parser.compile<3>();
    
        for (MFIter mfi(DMI,TilingIfNotGPU()); mfi.isValid(); ++mfi)
        {
            const Box& bx = mfi.tilebox(); // no ghost cells

            const Array4<Real>& DMI_arr = DMI.array(mfi);

            amrex::ParallelFor( bx, [=] AMREX_GPU_DEVICE (int i, int j, int k)
            {
                Real x = prob_lo[0] + (i+0.5) * dx[0];
                Real y = prob_lo[1] + (j+0.5) * dx[1];
                Real z = prob_lo[2] + (k+0.5) * dx[2];

                DMI_arr(i,j,k) = DMI_p(x,y,z);

            }); 
        }
    }

    if (anisotropy_coupling == 1) {
        std::string anisotropy_parser_string;
        pp.get("anisotropy_parser(x,y,z)",anisotropy_parser_string);
        Parser anisotropy_parser(anisotropy_parser_string);
        anisotropy_parser.registerVariables({"x","y","z"});
        auto anisotropy_p = anisotropy_parser.compile<3>();

        for (MFIter mfi(anisotropy,TilingIfNotGPU()); mfi.isValid(); ++mfi)
        {
            const Box& bx = mfi.tilebox(); // no ghost cells

            const Array4<Real>& anisotropy_arr = anisotropy.array(mfi);

            amrex::ParallelFor( bx, [=] AMREX_GPU_DEVICE (int i, int j, int k)
            {
                Real x = prob_lo[0] + (i+0.5) * dx[0];
                Real y = prob_lo[1] + (j+0.5) * dx[1];
                Real z = prob_lo[2] + (k+0.5) * dx[2];

                anisotropy_arr(i,j,k) = anisotropy_p(x,y,z);

            }); 
        }
    }

}

//Initialize fields
void InitializeFields(Array< MultiFab, AMREX_SPACEDIM >&  Mfield,
                      const       Geometry& geom)
{
    // timer for profiling
    BL_PROFILE_VAR("InitializeFields()",InitializeFields);

    ParmParse pp;

    std::string Mx_parser_string;
    pp.get("Mx_parser(x,y,z)",Mx_parser_string);
    Parser Mx_parser(Mx_parser_string);
    Mx_parser.registerVariables({"x","y","z"});
    auto Mx_p = Mx_parser.compile<3>();

    std::string My_parser_string;
    pp.get("My_parser(x,y,z)",My_parser_string);
    Parser My_parser(My_parser_string);
    My_parser.registerVariables({"x","y","z"});
    auto My_p = My_parser.compile<3>();

    std::string Mz_parser_string;
    pp.get("Mz_parser(x,y,z)",Mz_parser_string);
    Parser Mz_parser(Mz_parser_string);
    Mz_parser.registerVariables({"x","y","z"});
    auto Mz_p = Mz_parser.compile<3>();

    for (MFIter mfi(Mfield[0]); mfi.isValid(); ++mfi)
    {
        const Box& bx = mfi.growntilebox(1);
 
        // extract dx from the geometry object
        GpuArray<Real,AMREX_SPACEDIM> dx = geom.CellSizeArray();

        // extract field data
        const Array4<Real>& Mx = Mfield[0].array(mfi);         
        const Array4<Real>& My = Mfield[1].array(mfi);         
        const Array4<Real>& Mz = Mfield[2].array(mfi);

        amrex::ParallelFor( bx, [=] AMREX_GPU_DEVICE (int i, int j, int k)
        {

            Real x = prob_lo[0] + (i+0.5) * dx[0];
            Real y = prob_lo[1] + (j+0.5) * dx[1];
            Real z = prob_lo[2] + (k+0.5) * dx[2];

            Mx(i,j,k) = Mx_p(x,y,z);
            My(i,j,k) = My_p(x,y,z);
            Mz(i,j,k) = Mz_p(x,y,z);

        });

    }
}

void ComputeHbias(Array< MultiFab, AMREX_SPACEDIM >&  H_biasfield,
                  const           Real& time,
                  const       Geometry& geom)
{
    // timer for profiling
    BL_PROFILE_VAR("ComputeHbias()",ComputeHbias);

    ParmParse pp;

    std::string Hx_bias_parser_string;
    pp.get("Hx_bias_parser(x,y,z,t)",Hx_bias_parser_string);
    Parser Hx_bias_parser(Hx_bias_parser_string);
    Hx_bias_parser.registerVariables({"x","y","z","t"});
    auto Hx_bias_p = Hx_bias_parser.compile<4>();

    std::string Hy_bias_parser_string;
    pp.get("Hy_bias_parser(x,y,z,t)",Hy_bias_parser_string);
    Parser Hy_bias_parser(Hy_bias_parser_string);
    Hy_bias_parser.registerVariables({"x","y","z","t"});
    auto Hy_bias_p = Hy_bias_parser.compile<4>();

    std::string Hz_bias_parser_string;
    pp.get("Hz_bias_parser(x,y,z,t)",Hz_bias_parser_string);
    Parser Hz_bias_parser(Hz_bias_parser_string);
    Hz_bias_parser.registerVariables({"x","y","z","t"});
    auto Hz_bias_p = Hz_bias_parser.compile<4>();

    for (MFIter mfi(H_biasfield[0]); mfi.isValid(); ++mfi)
    {
        const Box& bx = mfi.tilebox();

        // extract dx from the geometry object
        GpuArray<Real,AMREX_SPACEDIM> dx = geom.CellSizeArray();

        // extract field data
        const Array4<Real>& Hx_bias = H_biasfield[0].array(mfi);
        const Array4<Real>& Hy_bias = H_biasfield[1].array(mfi);
        const Array4<Real>& Hz_bias = H_biasfield[2].array(mfi);

        amrex::ParallelFor( bx, [=] AMREX_GPU_DEVICE (int i, int j, int k)
        {
            Real x = prob_lo[0] + (i+0.5) * dx[0];
            Real y = prob_lo[1] + (j+0.5) * dx[1];
            Real z = prob_lo[2] + (k+0.5) * dx[2];

            Hx_bias(i,j,k) = Hx_bias_p(x,y,z,time);
            Hy_bias(i,j,k) = Hy_bias_p(x,y,z,time);
            Hz_bias(i,j,k) = Hz_bias_p(x,y,z,time);
        });
    }
}

void ComputeAlpha(MultiFab&  alpha,
                  const Geometry& geom,
                  const Real& time)
{
    // timer for profiling
    BL_PROFILE_VAR("ComputeAlpha()",ComputeAlpha);

    // extract dx from the geometry object
    GpuArray<Real,AMREX_SPACEDIM> dx = geom.CellSizeArray();

    ParmParse pp;

    std::string alpha_parser_string;
    pp.get("alpha_parser(x,y,z,t)",alpha_parser_string);
    Parser alpha_parser(alpha_parser_string);
    alpha_parser.registerVariables({"x","y","z","t"});
    auto alpha_p = alpha_parser.compile<4>();
    
    // loop over boxes
    for (MFIter mfi(alpha,TilingIfNotGPU); mfi.isValid(); ++mfi)
    {
        const Box& bx = mfi.tilebox();

        const Array4<Real>& alpha_arr = alpha.array(mfi);

        amrex::ParallelFor( bx, [=] AMREX_GPU_DEVICE (int i, int j, int k)
        {
            Real x = prob_lo[0] + (i+0.5) * dx[0];
            Real y = prob_lo[1] + (j+0.5) * dx[1];
            Real z = prob_lo[2] + (k+0.5) * dx[2];

            alpha_arr(i,j,k) = alpha_p(x,y,z,time);

        }); 
    }
}
