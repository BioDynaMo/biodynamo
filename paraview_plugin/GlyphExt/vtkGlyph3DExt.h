#ifndef PARAVIEW_PLUGIN_GLYPHEXT_VTKGLYPH3DEXT_H_
#define PARAVIEW_PLUGIN_GLYPHEXT_VTKGLYPH3DEXT_H_

#include "vtkGlyph3D.h"

#define VTK_SCALE_BY_NORMAL 4

class VTK_EXPORT vtkGlyph3DExt : public vtkGlyph3D {
 public:
  static vtkGlyph3DExt* New();
  vtkTypeMacro(vtkGlyph3DExt, vtkGlyph3D);
  void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;  // NOLINT

 protected:
  vtkGlyph3DExt();
  ~vtkGlyph3DExt();

  //@{
  /**
   * Method called in RequestData() to do the actual data processing. This will
   * glyph the \c input, filling up the \c output based on the filter
   * parameters.
   */
  bool Execute(vtkDataSet* input, vtkInformationVector* sourceVector,
               vtkPolyData* output) VTK_OVERRIDE;
  bool Execute(vtkDataSet* input, vtkInformationVector* sourceVector,
               vtkPolyData* output, vtkDataArray* inSScalars,
               vtkDataArray* inVectors) VTK_OVERRIDE;

 private:
  vtkGlyph3DExt(const vtkGlyph3DExt&) VTK_DELETE_FUNCTION;
  void operator=(const vtkGlyph3DExt&) VTK_DELETE_FUNCTION;
};

#endif  // PARAVIEW_PLUGIN_GLYPHEXT_VTKGLYPH3DEXT_H_
