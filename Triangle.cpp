#include "Triangle.h"
#include "Renderer.h"

Triangle::Triangle()
{
	vertices[0] = { 0.f,0.5f,0.f };
	vertices[1] = { 0.5f,-0.5f,0.f };
	vertices[2] = { -0.5f,-0.5f,0.f };
}

Triangle::~Triangle()
{
	DX_SAFE_RELEASE(vertexBuffer);
}

//	頂点バッファーの作成
bool Triangle::CreateVertexBuffer(Renderer& renderer)
{
	//	頂点バッファ定義の構造体を設定
	D3D11_BUFFER_DESC vertexBufferDesc = {};
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * 3;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	//	サブリソース定義をを頂点データで初期化
	D3D11_SUBRESOURCE_DATA vertexSubData;
	vertexSubData.pSysMem = vertices;

	//	CreateBufferメソッドで頂点バッファを取得
	auto hr = renderer.GetDevice()->CreateBuffer(
		&vertexBufferDesc,
		&vertexSubData,
		&vertexBuffer
	);

	if (FAILED(hr))return false;

	return true;
}

void Triangle::Draw(Renderer& renderer)
{
	auto pDeviceContext = renderer.GetDeviceContext();
	uint32_t strides[1] = { sizeof(Vertex) };
	uint32_t offsets[1] = { 0 };
	pDeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, strides, offsets);
	pDeviceContext->Draw(VERTEX_NUM, 0);
}