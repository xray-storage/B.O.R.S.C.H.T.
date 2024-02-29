#ifndef	xrD3DDefs_included
#define	xrD3DDefs_included
#pragma once

#ifdef	USE_DX10

class	dx10State;

typedef	ID3D11VertexShader		ID3DVertexShader;
typedef	ID3D11GeometryShader	ID3DGeometryShader;
typedef	ID3D11PixelShader		ID3DPixelShader;
typedef	ID3D10Blob				ID3DBlob;
typedef	D3D10_SHADER_MACRO		D3D_SHADER_MACRO;
typedef	ID3D11Query				ID3DQuery;
typedef	D3D11_VIEWPORT			D3D_VIEWPORT;
typedef	ID3D10Include			ID3DInclude;
typedef	ID3D11Texture2D			ID3DTexture2D;
typedef	ID3D11RenderTargetView	ID3DRenderTargetView;
typedef	ID3D11DepthStencilView	ID3DDepthStencilView;
typedef	ID3D11Resource			ID3DBaseTexture;
typedef	D3D11_TEXTURE2D_DESC	D3D_TEXTURE2D_DESC;
typedef ID3D11Buffer			ID3DVertexBuffer;
typedef ID3D11Buffer			ID3DIndexBuffer;
typedef	ID3D11Texture3D			ID3DTexture3D;
typedef	dx10State				ID3DState;

typedef D3DX10_IMAGE_FILE_FORMAT D3D_IMAGE_FILE_FORMAT;
const D3D_IMAGE_FILE_FORMAT D3DIFF_TGA = D3DX10_IFF_BMP;
const D3D_IMAGE_FILE_FORMAT D3DIFF_PNG = D3DX10_IFF_PNG;
const D3D_IMAGE_FILE_FORMAT D3DIFF_JPG = D3DX10_IFF_JPG;

#define DX10_ONLY(expr)			expr

#define HAS_GS
#ifdef HAS_GS
#define HAS_3DFLUID
#endif

#else	//	USE_DX10

typedef	IDirect3DVertexShader9	ID3DVertexShader;
typedef	IDirect3DPixelShader9	ID3DPixelShader;
typedef	ID3DXBuffer				ID3DBlob;
typedef	D3DXMACRO				D3D_SHADER_MACRO;
typedef	IDirect3DQuery9			ID3DQuery;
typedef	D3DVIEWPORT9			D3D_VIEWPORT;
typedef	ID3DXInclude			ID3DInclude;
typedef	IDirect3DTexture9		ID3DTexture2D;
typedef	IDirect3DSurface9		ID3DRenderTargetView;
typedef	IDirect3DSurface9		ID3DDepthStencilView;
typedef	IDirect3DBaseTexture9	ID3DBaseTexture;
typedef	D3DSURFACE_DESC			D3D_TEXTURE2D_DESC;
typedef IDirect3DVertexBuffer9	ID3DVertexBuffer;
typedef IDirect3DIndexBuffer9	ID3DIndexBuffer;
typedef	IDirect3DVolumeTexture9	ID3DTexture3D;
typedef	IDirect3DStateBlock9	ID3DState;

typedef D3DXIMAGE_FILEFORMAT	D3D_IMAGE_FILE_FORMAT;
const D3D_IMAGE_FILE_FORMAT D3DIFF_TGA = D3DXIFF_TGA;
const D3D_IMAGE_FILE_FORMAT D3DIFF_PNG = D3DXIFF_PNG;
const D3D_IMAGE_FILE_FORMAT D3DIFF_JPG = D3DXIFF_JPG;

#define DX10_ONLY(expr)			do {} while (0)

#endif	//	USE_DX10


#endif	//	xrD3DDefs_included