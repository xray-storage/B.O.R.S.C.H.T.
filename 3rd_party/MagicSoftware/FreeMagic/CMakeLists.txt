cmake_minimum_required(VERSION 3.15)
project(FreeMagic LANGUAGES C CXX)

add_library(MagicFM_Core STATIC)
target_include_directories(MagicFM_Core PUBLIC Include)
target_sources(MagicFM_Core PRIVATE
	Source/Core/MgcColorRGB.cpp
	Source/Core/MgcColorRGB.h
	Source/Core/MgcColorRGB.inl
	Source/Core/MgcCommand.cpp
	Source/Core/MgcCommand.h
	Source/Core/MgcEdgeMesh.cpp
	Source/Core/MgcEdgeMesh.h
	Source/Core/MgcEdgeMesh.inl
	Source/Core/MgcIntervalIntersection.cpp
	Source/Core/MgcIntervalIntersection.h
	Source/Core/MgcMath.cpp
	Source/Core/MgcMath.h
	Source/Core/MgcMath.inl
	Source/Core/MgcMTEdge.cpp
	Source/Core/MgcMTEdge.h
	Source/Core/MgcMTEdge.inl
	Source/Core/MgcMTIEdge.h
	Source/Core/MgcMTIEdge.inl
	Source/Core/MgcMTITriangle.h
	Source/Core/MgcMTITriangle.inl
	Source/Core/MgcMTIVertex.h
	Source/Core/MgcMTIVertex.inl
	Source/Core/MgcMTMesh.cpp
	Source/Core/MgcMTMesh.h
	Source/Core/MgcMTMesh.inl
	Source/Core/MgcMTTriangle.cpp
	Source/Core/MgcMTTriangle.h
	Source/Core/MgcMTTriangle.inl
	Source/Core/MgcMTVertex.cpp
	Source/Core/MgcMTVertex.h
	Source/Core/MgcMTVertex.inl
	Source/Core/MgcPolynomial.cpp
	Source/Core/MgcPolynomial.h
	Source/Core/MgcSmallSet.h
	Source/Core/MgcSmallSet.inl
	Source/Core/MgcTriangleMesh.cpp
	Source/Core/MgcTriangleMesh.h
	Source/Core/MgcTriangleMesh.inl
	Source/Core/MgcTUnorderedSet.h
	Source/Core/MgcTUnorderedSet.inl)

add_library(MagicFM_Distance2D STATIC)
target_include_directories(MagicFM_Distance2D PUBLIC Include)
target_sources(MagicFM_Distance2D PRIVATE
	Source/Distance2D/MgcDist2DVecElp.cpp
	Source/Distance2D/MgcDist2DVecElp.h
	Source/Distance2D/MgcDist2DVecQdr.cpp
	Source/Distance2D/MgcDist2DVecQdr.h)

add_library(MagicFM_Distance3D STATIC)
target_include_directories(MagicFM_Distance3D PUBLIC Include)
target_sources(MagicFM_Distance3D PRIVATE
	Source/Distance3D/MgcDist3DCirCir.cpp
	Source/Distance3D/MgcDist3DCirCir.h
	Source/Distance3D/MgcDist3DLinBox.cpp
	Source/Distance3D/MgcDist3DLinBox.h
	Source/Distance3D/MgcDist3DLinCir.cpp
	Source/Distance3D/MgcDist3DLinCir.h
	Source/Distance3D/MgcDist3DLinLin.cpp
	Source/Distance3D/MgcDist3DLinLin.h
	Source/Distance3D/MgcDist3DLinPgm.cpp
	Source/Distance3D/MgcDist3DLinPgm.h
	Source/Distance3D/MgcDist3DLinRct.cpp
	Source/Distance3D/MgcDist3DLinRct.h
	Source/Distance3D/MgcDist3DLinTri.cpp
	Source/Distance3D/MgcDist3DLinTri.h
	Source/Distance3D/MgcDist3DPgmPgm.cpp
	Source/Distance3D/MgcDist3DPgmPgm.h
	Source/Distance3D/MgcDist3DRctRct.cpp
	Source/Distance3D/MgcDist3DRctRct.h
	Source/Distance3D/MgcDist3DTriPgm.cpp
	Source/Distance3D/MgcDist3DTriPgm.h
	Source/Distance3D/MgcDist3DTriRct.cpp
	Source/Distance3D/MgcDist3DTriRct.h
	Source/Distance3D/MgcDist3DTriTri.cpp
	Source/Distance3D/MgcDist3DTriTri.h
	Source/Distance3D/MgcDist3DVecBox.cpp
	Source/Distance3D/MgcDist3DVecBox.h
	Source/Distance3D/MgcDist3DVecCir.cpp
	Source/Distance3D/MgcDist3DVecCir.h
	Source/Distance3D/MgcDist3DVecElp.cpp
	Source/Distance3D/MgcDist3DVecElp.h
	Source/Distance3D/MgcDist3DVecFrustum.cpp
	Source/Distance3D/MgcDist3DVecFrustum.h
	Source/Distance3D/MgcDist3DVecLin.cpp
	Source/Distance3D/MgcDist3DVecLin.h
	Source/Distance3D/MgcDist3DVecPgm.cpp
	Source/Distance3D/MgcDist3DVecPgm.h
	Source/Distance3D/MgcDist3DVecPln.cpp
	Source/Distance3D/MgcDist3DVecPln.h
	Source/Distance3D/MgcDist3DVecQdr.cpp
	Source/Distance3D/MgcDist3DVecQdr.h
	Source/Distance3D/MgcDist3DVecRct.cpp
	Source/Distance3D/MgcDist3DVecRct.h
	Source/Distance3D/MgcDist3DVecTri.cpp
	Source/Distance3D/MgcDist3DVecTri.h
	Source/Distance3D/MgcDistance3D.pkg
	Source/Distance3D/MgcEllipsoidGeodesicDist.cpp
	Source/Distance3D/MgcEllipsoidGeodesicDist.h)

add_library(MagicFM_Core2D STATIC)
target_include_directories(MagicFM_Core2D PUBLIC Include)
target_sources(MagicFM_Core2D PRIVATE
	Source/Core2D/MgcArc2.h
	Source/Core2D/MgcArc2.inl
	Source/Core2D/MgcBox2.cpp
	Source/Core2D/MgcBox2.h
	Source/Core2D/MgcBox2.inl
	Source/Core2D/MgcCircle2.h
	Source/Core2D/MgcCircle2.inl
	Source/Core2D/MgcDisk2.h
	Source/Core2D/MgcDisk2.inl
	Source/Core2D/MgcEllipse2.h
	Source/Core2D/MgcEllipse2.inl
	Source/Core2D/MgcLine2.h
	Source/Core2D/MgcLine2.inl
	Source/Core2D/MgcMatrix2.cpp
	Source/Core2D/MgcMatrix2.h
	Source/Core2D/MgcMatrix2.inl
	Source/Core2D/MgcRay2.h
	Source/Core2D/MgcRay2.inl
	Source/Core2D/MgcSegment2.h
	Source/Core2D/MgcSegment2.inl
	Source/Core2D/MgcTriangle2.h
	Source/Core2D/MgcTriangle2.inl
	Source/Core2D/MgcVector2.cpp
	Source/Core2D/MgcVector2.h
	Source/Core2D/MgcVector2.inl)

add_library(MagicFM_Core3D STATIC)
target_include_directories(MagicFM_Core3D PUBLIC Include)
target_sources(MagicFM_Core3D PRIVATE
	Source/Core3D/MgcBox3.cpp
	Source/Core3D/MgcBox3.h
	Source/Core3D/MgcBox3.inl
	Source/Core3D/MgcCapsule.h
	Source/Core3D/MgcCapsule.inl
	Source/Core3D/MgcCircle3.h
	Source/Core3D/MgcCircle3.inl
	Source/Core3D/MgcCone3.h
	Source/Core3D/MgcCone3.inl
	Source/Core3D/MgcCylinder.h
	Source/Core3D/MgcCylinder.inl
	Source/Core3D/MgcDisk3.h
	Source/Core3D/MgcDisk3.inl
	Source/Core3D/MgcEllipsoid.h
	Source/Core3D/MgcEllipsoid.inl
	Source/Core3D/MgcExtraSpin.cpp
	Source/Core3D/MgcExtraSpin.h
	Source/Core3D/MgcExtraSpin.inl
	Source/Core3D/MgcFrustum.cpp
	Source/Core3D/MgcFrustum.h
	Source/Core3D/MgcFrustum.inl
	Source/Core3D/MgcLine3.h
	Source/Core3D/MgcLine3.inl
	Source/Core3D/MgcLozenge.h
	Source/Core3D/MgcLozenge.inl
	Source/Core3D/MgcMatrix3.cpp
	Source/Core3D/MgcMatrix3.h
	Source/Core3D/MgcMatrix3.inl
	Source/Core3D/MgcParallelogram3.h
	Source/Core3D/MgcParallelogram3.inl
	Source/Core3D/MgcPlane.cpp
	Source/Core3D/MgcPlane.h
	Source/Core3D/MgcPlane.inl
	Source/Core3D/MgcQuaternion.cpp
	Source/Core3D/MgcQuaternion.h
	Source/Core3D/MgcRay3.h
	Source/Core3D/MgcRay3.inl
	Source/Core3D/MgcRectangle3.h
	Source/Core3D/MgcRectangle3.inl
	Source/Core3D/MgcSegment3.h
	Source/Core3D/MgcSegment3.inl
	Source/Core3D/MgcSphere.h
	Source/Core3D/MgcSphere.inl
	Source/Core3D/MgcTetrahedron.cpp
	Source/Core3D/MgcTetrahedron.h
	Source/Core3D/MgcTetrahedron.inl
	Source/Core3D/MgcTorus.cpp
	Source/Core3D/MgcTorus.h
	Source/Core3D/MgcTorus.inl
	Source/Core3D/MgcTriangle3.h
	Source/Core3D/MgcTriangle3.inl
	Source/Core3D/MgcVector3.cpp
	Source/Core3D/MgcVector3.h
	Source/Core3D/MgcVector3.inl
	Source/Core3D/MgcXFormInterp.cpp
	Source/Core3D/MgcXFormInterp.h)

add_library(MagicFM SHARED)
target_include_directories(MagicFM PUBLIC Include)
target_compile_definitions(MagicFM PRIVATE MAGICFMDLL_EXPORTS)
target_sources(MagicFM PRIVATE
	Source/Approximation2D/MgcAppr2DCircleFit.cpp
	Source/Approximation2D/MgcAppr2DCircleFit.h
	Source/Approximation2D/MgcAppr2DEllipseByArcs.cpp
	Source/Approximation2D/MgcAppr2DEllipseByArcs.h
	Source/Approximation2D/MgcAppr2DEllipseFit.cpp
	Source/Approximation2D/MgcAppr2DEllipseFit.h
	Source/Approximation2D/MgcAppr2DGaussPointsFit.cpp
	Source/Approximation2D/MgcAppr2DGaussPointsFit.h
	Source/Approximation2D/MgcAppr2DLineFit.cpp
	Source/Approximation2D/MgcAppr2DLineFit.h
	Source/Approximation2D/MgcAppr2DQuadraticFit.cpp
	Source/Approximation2D/MgcAppr2DQuadraticFit.h

	Source/Approximation3D/MgcAppr3DEllipsoidFit.cpp
	Source/Approximation3D/MgcAppr3DEllipsoidFit.h
	Source/Approximation3D/MgcAppr3DGaussPointsFit.cpp
	Source/Approximation3D/MgcAppr3DGaussPointsFit.h
	Source/Approximation3D/MgcAppr3DLineFit.cpp
	Source/Approximation3D/MgcAppr3DLineFit.h
	Source/Approximation3D/MgcAppr3DParaboloidFit.cpp
	Source/Approximation3D/MgcAppr3DParaboloidFit.h
	Source/Approximation3D/MgcAppr3DPlaneFit.cpp
	Source/Approximation3D/MgcAppr3DPlaneFit.h
	Source/Approximation3D/MgcAppr3DQuadraticFit.cpp
	Source/Approximation3D/MgcAppr3DQuadraticFit.h
	Source/Approximation3D/MgcAppr3DSphereFit.cpp
	Source/Approximation3D/MgcAppr3DSphereFit.h
	
	Source/Containment2D/MgcCont2DBox.cpp
	Source/Containment2D/MgcCont2DBox.h
	Source/Containment2D/MgcCont2DCircleScribe.cpp
	Source/Containment2D/MgcCont2DCircleScribe.h
	Source/Containment2D/MgcCont2DEllipse.cpp
	Source/Containment2D/MgcCont2DEllipse.h
	Source/Containment2D/MgcCont2DMinBox.cpp
	Source/Containment2D/MgcCont2DMinBox.h
	Source/Containment2D/MgcCont2DMinCircle.cpp
	Source/Containment2D/MgcCont2DMinCircle.h
	Source/Containment2D/MgcCont2DPointInPolygon.cpp
	Source/Containment2D/MgcCont2DPointInPolygon.h
	Source/Containment2D/MgcConvexHull2D.cpp
	Source/Containment2D/MgcConvexHull2D.h
	Source/Containment2D/MgcConvexHull2D.inl
	Source/Containment2D/MgcMinAreaEllipseCR.cpp
	Source/Containment2D/MgcMinAreaEllipseCR.h
	Source/Containment2D/MgcSeparatePointSets2D.cpp
	Source/Containment2D/MgcSeparatePointSets2D.h

	Source/Containment3D/MgcCont3DBox.cpp
	Source/Containment3D/MgcCont3DBox.h
	Source/Containment3D/MgcCont3DCapsule.cpp
	Source/Containment3D/MgcCont3DCapsule.h
	Source/Containment3D/MgcCont3DCircleSphereScribe.cpp
	Source/Containment3D/MgcCont3DCircleSphereScribe.h
	Source/Containment3D/MgcCont3DCylinder.cpp
	Source/Containment3D/MgcCont3DCylinder.h
	Source/Containment3D/MgcCont3DEllipsoid.cpp
	Source/Containment3D/MgcCont3DEllipsoid.h
	Source/Containment3D/MgcCont3DLozenge.cpp
	Source/Containment3D/MgcCont3DLozenge.h
	Source/Containment3D/MgcCont3DMinBox.cpp
	Source/Containment3D/MgcCont3DMinBox.h
	Source/Containment3D/MgcCont3DMinSphere.cpp
	Source/Containment3D/MgcCont3DMinSphere.h
	Source/Containment3D/MgcCont3DSphere.cpp
	Source/Containment3D/MgcCont3DSphere.h
	Source/Containment3D/MgcConvexHull3D.cpp
	Source/Containment3D/MgcConvexHull3D.h
	Source/Containment3D/MgcConvexHull3D.inl
	Source/Containment3D/MgcMinVolumeEllipsoidCR.cpp
	Source/Containment3D/MgcMinVolumeEllipsoidCR.h
	Source/Containment3D/MgcSeparatePointSets3D.cpp
	Source/Containment3D/MgcSeparatePointSets3D.h
	
	Source/Numerics/MgcBisect1.cpp
	Source/Numerics/MgcBisect1.h
	Source/Numerics/MgcBisect2.cpp
	Source/Numerics/MgcBisect2.h
	Source/Numerics/MgcBisect3.cpp
	Source/Numerics/MgcBisect3.h
	Source/Numerics/MgcEigen.cpp
	Source/Numerics/MgcEigen.h
	Source/Numerics/MgcEigen.inl
	Source/Numerics/MgcEuler.cpp
	Source/Numerics/MgcEuler.h
	Source/Numerics/MgcEuler.inl
	Source/Numerics/MgcFastFunction.cpp
	Source/Numerics/MgcFastFunction.h
	Source/Numerics/MgcIntegrate.cpp
	Source/Numerics/MgcIntegrate.h
	Source/Numerics/MgcLinearSystem.cpp
	Source/Numerics/MgcLinearSystem.h
	Source/Numerics/MgcLinearSystem.inl
	Source/Numerics/MgcMidpoint.cpp
	Source/Numerics/MgcMidpoint.h
	Source/Numerics/MgcMidpoint.inl
	Source/Numerics/MgcMinimize1D.cpp
	Source/Numerics/MgcMinimize1D.h
	Source/Numerics/MgcMinimize1D.inl
	Source/Numerics/MgcMinimizeND.cpp
	Source/Numerics/MgcMinimizeND.h
	Source/Numerics/MgcMinimizeND.inl
	Source/Numerics/MgcNumerics.pkg
	Source/Numerics/MgcODE.cpp
	Source/Numerics/MgcODE.h
	Source/Numerics/MgcODE.inl
	Source/Numerics/MgcRK4.cpp
	Source/Numerics/MgcRK4.h
	Source/Numerics/MgcRK4.inl
	Source/Numerics/MgcRK4Adapt.cpp
	Source/Numerics/MgcRK4Adapt.h
	Source/Numerics/MgcRK4Adapt.inl
	Source/Numerics/MgcSpecialFunction.cpp
	Source/Numerics/MgcSpecialFunction.h)
target_link_libraries(MagicFM PRIVATE MagicFM_Core MagicFM_Distance2D MagicFM_Distance3D MagicFM_Core2D MagicFM_Core3D)
add_library(MagicFM::MagicFM ALIAS MagicFM)
