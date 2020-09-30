#ifndef	dx10ShaderResourceStateCache_included
#define	dx10ShaderResourceStateCache_included
#pragma once

class dx10ShaderResourceStateCache
{
public:
	dx10ShaderResourceStateCache();

	void	ResetDeviceState();

	void	Apply();

	void	SetPSResource( u32 uiSlot, ID3D11ShaderResourceView	*pRes );
#ifdef	HAS_GS
	void	SetGSResource( u32 uiSlot, ID3D11ShaderResourceView	*pRes );
#endif
	void	SetVSResource( u32 uiSlot, ID3D11ShaderResourceView	*pRes );

private:
	ID3D11ShaderResourceView	*m_PSViews[CBackend::mtMaxPixelShaderTextures];
#ifdef	HAS_GS
	ID3D11ShaderResourceView	*m_GSViews[CBackend::mtMaxGeometryShaderTextures];
#endif
	ID3D11ShaderResourceView	*m_VSViews[CBackend::mtMaxVertexShaderTextures];

	u32		m_uiMinPSView;
	u32		m_uiMaxPSView;
#ifdef	HAS_GS
	u32		m_uiMinGSView;
	u32		m_uiMaxGSView;
#endif
	u32		m_uiMinVSView;
	u32		m_uiMaxVSView;

	bool	m_bUpdatePSViews;
#ifdef	HAS_GS
	bool	m_bUpdateGSViews;
#endif
	bool	m_bUpdateVSViews;
};

extern	dx10ShaderResourceStateCache	SRVSManager;

#endif	//	dx10ShaderResourceStateCache_included