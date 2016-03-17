/**
 * This file enables transparent type conversions between cx3d::Color (C++)
 * and java.awt.Color (Java)
 */

%typemap(jstype) cx3d::Color "java.awt.Color"

// helper functions
%pragma(java) modulecode=%{
  static java.awt.Color convertColor(long cPtr, boolean cMemoryOwn) {
    long color = new ini.cx3d.swig.physics.Color(cPtr, cMemoryOwn).getValue();
    return new java.awt.Color((int) color, true);
  }

  static ini.cx3d.swig.physics.Color convertColor(java.awt.Color color) {
    return new ini.cx3d.swig.physics.Color(color.getRGB());
  }
%}

// return typemap
%typemap(javaout) cx3d::Color {
  return $module.convertColor($jnicall, $owner);
}

%typemap(javain,
        pre = "    Color temp$javainput = $module.convertColor($javainput);",
        pgcppname="temp$javainput") cx3d::Color "$javaclassname.getCPtr(temp$javainput)"

%typemap(javadirectorin) cx3d::Color "$module.convertColor($jniinput, false)"

%typemap(javadirectorout) cx3d::Color "Color.getCPtr( physics.convertColor($javacall))"

// for the JNI descriptor define:
%typemap(directorin, descriptor="Ljava/awt/Color;") cx3d::Color, cx3d::Color& %{
  *(cx3d::Color **)&j$1 = (cx3d::Color *) &$1;
%}
