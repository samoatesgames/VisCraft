#include "cgizmo.h"

/*
 *	\brief Class constructor
*/
CGizmo::CGizmo()
{
	m_vertexBuffer = nullptr;
	m_indexBuffer = nullptr;

	m_vertexShader = nullptr;
	m_pixelShader = nullptr;
	m_layout = nullptr;
	m_matrixBuffer = nullptr;
	m_gizmoBuffer = nullptr;

	m_position = D3DXVECTOR3(0, 4, 0);

	m_gizmoState = GizmoState::Free;
}

/*
 *	\brief Class destructor
*/
CGizmo::~CGizmo()
{
	SafeRelease(m_indexBuffer);
	SafeRelease(m_vertexBuffer);
}

/*
 *	\brief Create the gizmo
*/
bool CGizmo::Create( 
		CRenderer *renderer						//!< Pointer to the renderer
	)
{
	m_renderer = renderer;

	m_indexCount = m_vertexCount = 3;

	// Create the vertex array.
	CGizmo::Vertex *const vertices = new CGizmo::Vertex[m_vertexCount];

	// Create the index array.
	unsigned long *const indices = new unsigned long[m_indexCount];

	// Load the vertex array with data.
	vertices[0].position = D3DXVECTOR3(-1.0f, -1.0f, 0.0f);		// Bottom left.
	vertices[1].position = D3DXVECTOR3(0.0f, 1.0f, 0.0f);		// Top middle.
	vertices[2].position = D3DXVECTOR3(1.0f, -1.0f, 0.0f);		// Bottom right.

	// Load the index array with data.
	indices[0] = 0;  // Bottom left.
	indices[1] = 1;  // Top middle.
	indices[2] = 2;  // Bottom right.

	// Set up the description of the static vertex buffer.
	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(CGizmo::Vertex) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	D3D11_SUBRESOURCE_DATA vertexData;
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
	if (FAILED(m_renderer->GetDevice()->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer)))
		return false;

	// Set up the description of the static index buffer.
	D3D11_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	if (FAILED(m_renderer->GetDevice()->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer)))
		return false;

	// Release the arrays now that the vertex and index buffers have been created and loaded.
	delete[] vertices;
	delete[] indices;

	// Compile the vertex shader code.
	HRESULT result;
	ID3D10Blob* errorMessage = nullptr;
	ID3D10Blob* vertexShaderBuffer = nullptr;
	result = D3DX11CompileFromFile("shaders/gizmo.vs", NULL, NULL, "ColorVertexShader", "vs_4_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, &vertexShaderBuffer, &errorMessage, NULL);
	if (FAILED(result))
		return false;
	
	// Compile the pixel shader code.
	ID3D10Blob* pixelShaderBuffer = nullptr;
	result = D3DX11CompileFromFile("shaders/gizmo.ps", NULL, NULL, "ColorPixelShader", "ps_4_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, &pixelShaderBuffer, &errorMessage, NULL);
	if (FAILED(result))
		return false;

	// Create the vertex shader from the buffer.
	result = m_renderer->GetDevice()->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
	if (FAILED(result))
		return false;

	// Create the pixel shader from the buffer.
	result = m_renderer->GetDevice()->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
	if (FAILED(result))
		return false;

	// Now setup the layout of the data that goes into the shader.
	// This setup needs to match the VertexType stucture in the ModelClass and in the shader.
	D3D11_INPUT_ELEMENT_DESC polygonLayout[1];
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	// Get a count of the elements in the layout.
	unsigned int numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// Create the vertex input layout.
	result = m_renderer->GetDevice()->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_layout);
	if (FAILED(result))
		return false;

	// Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
	SafeRelease(vertexShaderBuffer);
	SafeRelease(pixelShaderBuffer);

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	D3D11_BUFFER_DESC matrixBufferDesc;
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(CGizmo::MatrixBuffer);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = m_renderer->GetDevice()->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
	if (FAILED(result))
		return false;

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	D3D11_BUFFER_DESC gizmoBufferDesc;
	gizmoBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	gizmoBufferDesc.ByteWidth = sizeof(CGizmo::GizmoBuffer);
	gizmoBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	gizmoBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	gizmoBufferDesc.MiscFlags = 0;
	gizmoBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = m_renderer->GetDevice()->CreateBuffer(&gizmoBufferDesc, NULL, &m_gizmoBuffer);
	if (FAILED(result))
		return false;

	return true;
}

/*
 *	\brief Render the gizmo
*/
void CGizmo::Render(
		D3DXMATRIX worldMatrix,					//!< 
		D3DXMATRIX viewMatrix,					//!< 
		D3DXMATRIX projectionMatrix				//!< 
	)
{
	// Set vertex buffer stride and offset.
	unsigned int stride = sizeof(CGizmo::Vertex); 
	unsigned int offset = 0;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	m_renderer->GetDeviceContext()->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

	// Set the index buffer to active in the input assembler so it can be rendered.
	m_renderer->GetDeviceContext()->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	m_renderer->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// move to the position, but offset the y coordinate by one, to compensate for it been drawn around 0, 0, 0 local space
	D3DXMatrixTranslation(&worldMatrix, m_position.x, m_position.y + 1.0f, m_position.z); 
	
	// Transpose the matrices to prepare them for the shader.
	D3DXMatrixTranspose(&worldMatrix, &worldMatrix);
	D3DXMatrixTranspose(&viewMatrix, &viewMatrix);
	D3DXMatrixTranspose(&projectionMatrix, &projectionMatrix);	

	// Lock the constant buffer so it can be written to.
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	result = m_renderer->GetDeviceContext()->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return;
	
	// Get a pointer to the data in the constant buffer.
	CGizmo::MatrixBuffer *const matrixDataPtr = (CGizmo::MatrixBuffer*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	matrixDataPtr->world = worldMatrix;
	matrixDataPtr->view = viewMatrix;
	matrixDataPtr->projection = projectionMatrix;

	// Unlock the constant buffer.
	m_renderer->GetDeviceContext()->Unmap(m_matrixBuffer, 0);

	result = m_renderer->GetDeviceContext()->Map(m_gizmoBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return;

	// Get a pointer to the data in the constant buffer.
	CGizmo::GizmoBuffer *const gizmoDataPtr = (CGizmo::GizmoBuffer*)mappedResource.pData;

	// Copy the data into the constant buffer.

	// color based upon gizmo state
	if (m_gizmoState == GizmoState::Free)
	{
		gizmoDataPtr->color = D3DXVECTOR4(0.0f, 1.0f, 0.0f, 1.0f);
	}
	else
	{
		gizmoDataPtr->color = D3DXVECTOR4(1.0f, 0.0f, 0.0f, 1.0f);
	}

	// Unlock the constant buffer.
	m_renderer->GetDeviceContext()->Unmap(m_gizmoBuffer, 0);

	// Finally set the constant buffer in the vertex shader with the updated values.
	m_renderer->GetDeviceContext()->VSSetConstantBuffers(0, 1, &m_matrixBuffer);
	m_renderer->GetDeviceContext()->VSSetConstantBuffers(1, 1, &m_gizmoBuffer);

	// Set the vertex input layout.
	m_renderer->GetDeviceContext()->IASetInputLayout(m_layout);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	m_renderer->GetDeviceContext()->VSSetShader(m_vertexShader, NULL, 0);
	m_renderer->GetDeviceContext()->PSSetShader(m_pixelShader, NULL, 0);

	// Render the triangle.
	m_renderer->GetDeviceContext()->DrawIndexed(m_indexCount, 0, 0);
}

/*
*	\breif 
*/
void CGizmo::Control( 
		CInput *input,									//!< 
		CTerrain *terrain								//!<
	)
{
	if (input->IsMouseDown(MouseButton::Right))
	{
		if (m_gizmoState != GizmoState::Locked)
		{
			const D3DXVECTOR2 mousePos = input->GetMousePosition();
			m_dragData.startY = mousePos.y;
			m_gizmoState = GizmoState::Locked;
		}		
	}
	else
	{
		m_gizmoState = GizmoState::Free;
	}

	if (m_gizmoState == GizmoState::Free)
	{
		const D3DXVECTOR2 mousePos = input->GetMousePosition();
		m_position.x = mousePos.x * 0.1f;
		m_position.z = mousePos.y * 0.1f;
		m_position.y = terrain->GetTerrainHeightAt(m_position.x, m_position.z);
	}
	else
	{
		const D3DXVECTOR2 mousePos = input->GetMousePosition();
		const float moveAmount = (mousePos.y - m_dragData.startY) * 0.1f;

		if (moveAmount == 0.0f) return;

		for (int xOffset = -2; xOffset <= 2; ++xOffset)
		{
			for (int zOffset = -2; zOffset <= 2; ++zOffset)
			{
				HeightMap *hmap = terrain->GetTerrainVertexAt(m_position.x + xOffset, m_position.z + zOffset);
				hmap->position.y -= moveAmount;
			}
		}

		terrain->UpdateHeightMap();
		m_dragData.startY = mousePos.y;
	}
}
