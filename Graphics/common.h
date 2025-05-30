// Common parameters

//#define MAX_PARTICLES 65536
#define MAX_PARTICLES 262144

//#define MAX_PARTICLES_PER_CELL 16 // Only viable when using Mullet.M position based fluid technique
#define MAX_PARTICLES_PER_CELL 32

#define WORKGROUP_SIZE_X 1024

#define COMPUTE_CELLS_PER_WORKGROUP 16

#define PROJECTIONVIEW_UBO 0

#define FLUID_CONFIG_UBO 1
#define FLUID_DATA_SSBO 2

