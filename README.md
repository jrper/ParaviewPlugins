Source files for basic ParaView plugins.

ShowCVs : Show control volumes, i.e. show point data as cell data on a Voronoi dual data structure.
VelocitiesOnCVs :  Interpolate vector values onto the face centres of the control volume data structure.
GMSHreader : Pull the data from a gmsh file. Currently limited to simplicies, but only due to laziness in coding.
GMSHwriter : Output mesh to a gmsh .msh file. Data not included.
FLMLreader : Pull and display the mesh from a .flml file. One day this will initialize the data as well.
detectorreader: Reads the data from a Fluidity .detector file and outputs a vtkPolyData object.
statreader: Reads a Fluidity .stat file into a vtkTable object.
SplitBcs : Split high and low dimension data from an unstructured grid containing both, e.g. as produced by the GMSHreader plugin above.
MergePointsFilter:Map unstructured grids between continuous and discontinuous (also high and low order).
Supermesh: Use Fluidity femtools library to generate supermesh from a pair of unstructured meshes.
TemporalDataReaders: Get Fluidity style time data out of files and use for the time level.
