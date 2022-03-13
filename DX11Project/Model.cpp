#include "Model.h"

Model::Model() : Model(Mesh(), nullptr)
{

}

Model::Model(Mesh mesh, Material* material) : mesh(mesh), material(material), position({ 0,0,0 }), rotation({ 0,0,0 }), enabled(true)
{

}

void Model::Draw(ID3D11DeviceContext* context)
{
	UINT stride = sizeof(VERTEX);
	UINT offset = 0;

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetVertexBuffers(0, 1, &mesh.vertexBuffer, &stride, &offset);
	context->IASetIndexBuffer(mesh.indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	context->DrawIndexed(mesh.indicies.size(), 0, 0);
}

dx::XMMATRIX Model::GetModelMatrix()
{
	return dx::XMMatrixMultiply(dx::XMMatrixTranslationFromVector(this->position), dx::XMMatrixRotationRollPitchYawFromVector(this->rotation));
}

void Model::SetMesh(Mesh mesh)
{
	this->mesh = mesh;

}
