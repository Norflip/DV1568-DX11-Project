#include "Ray.h"

Ray::Ray(dx::XMVECTOR origin, dx::XMVECTOR direction) : origin(origin), direction(dx::XMVector3Normalize(direction))
{
	dx::XMFLOAT3 invf3;
	inv = dx::XMVectorDivide(dx::XMVectorSplatOne(), dx::XMVector3Normalize(direction));
	dx::XMStoreFloat3(&invf3, inv);
}

RayHit Ray::IntersectsModel(Model* model)
{
	if (model->ignorePicking)
		return RayHit();

	RayHit hit;

	if (IntersectsSphere(model->position, model->GetMesh().radius).intersected)
		hit = IntersectsMesh(model->GetMesh(), model->GetModelMatrix());

	return hit;
}

RayHit Ray::IntersectsSphere(dx::XMVECTOR center, float radius)
{
	RayHit hit;
	
	dx::XMVECTOR oc = dx::XMVectorSubtract(origin, center);
	float b = dx::XMVectorGetByIndex(dx::XMVector3Dot(oc, direction), 0) * 2.0f;
	float c = dx::XMVectorGetByIndex(dx::XMVector3Dot(oc, oc), 0) - (radius * radius);
	float d = b * b - c * 4;

	hit.intersected = !(d < 0);
	return hit;
}

RayHit Ray::IntersectsMesh(const Mesh& mesh, dx::XMMATRIX worldMatrix) const
{
	dx::XMVECTOR a, b, c;
	size_t triCount = mesh.indicies.size() / 3;
	RayHit hit;
	hit.intersected = false;

	for (size_t i = 0; i < triCount && !hit.intersected; i++)
	{
		a = dx::XMLoadFloat3(&mesh.vertexes[mesh.indicies[i * 3 + 0]].position);
		b = dx::XMLoadFloat3(&mesh.vertexes[mesh.indicies[i * 3 + 1]].position);
		c = dx::XMLoadFloat3(&mesh.vertexes[mesh.indicies[i * 3 + 2]].position);

		hit = IntersectsTriangle(
			dx::XMVector3Transform(a, worldMatrix),
			dx::XMVector3Transform(b, worldMatrix),
			dx::XMVector3Transform(c, worldMatrix)
		);
	}

	return hit;
}

RayHit Ray::IntersectsTriangle(const dx::XMVECTOR& a, const dx::XMVECTOR& b, const dx::XMVECTOR& c) const
{
	RayHit hit;
	hit.intersected = false;
	hit.normal = hit.position = dx::XMVectorZero();

	dx::XMVECTOR u = dx::XMVectorSubtract(b, a);
	dx::XMVECTOR v = dx::XMVectorSubtract(c, a);

	dx::XMVECTOR uxv = dx::XMVector3Cross(u, v);
	hit.normal = uxv;

	// kolla om triangel är synlig, gör en early exit annars.
	dx::XMVECTOR d = dx::XMVector3Dot(uxv, this->direction);

	if (d.m128_f32[0] < -0.0001f)
	{
		dx::XMVECTOR center = dx::XMVectorDivide(dx::XMVectorAdd(dx::XMVectorAdd(a, b), c), { 3,3,3 });
		dx::XMVECTOR po = dx::XMVectorSubtract(center, origin);
		dx::XMVECTOR t = dx::XMVectorDivide(dx::XMVector3Dot(po, uxv), d);

		float dt = t.m128_f32[0];
		hit.t = dt;

		// kolla om t är positiv
		if (dt <= 0.0f)
			return hit;

		dx::XMVECTOR p = dx::XMVectorAdd(origin, dx::XMVectorScale(direction, dt));

		hit.position = p;

		dx::XMVECTOR wP = dx::XMVectorSubtract(p, a);
		dx::XMVECTOR vxw = dx::XMVector3Cross(v, wP);
		dx::XMVECTOR vxu = dx::XMVectorScale(uxv, -1.0f);

		if (dx::XMVector3Less(dx::XMVector3Dot(vxw, vxu), dx::XMVectorZero()))
			return hit;

		dx::XMVECTOR uxw = dx::XMVector3Cross(u, wP);
		if (dx::XMVector3Less(dx::XMVector3Dot(uxw, uxv), dx::XMVectorZero()))
			return hit;

		dx::XMFLOAT3 uxvf3, vxwf3, uxwf3;
		dx::XMStoreFloat3(&uxvf3, uxv);
		dx::XMStoreFloat3(&vxwf3, vxw);
		dx::XMStoreFloat3(&uxwf3, uxw);

		float ds = sqrtf(uxvf3.x * uxvf3.x + uxvf3.y * uxvf3.y + uxvf3.z * uxvf3.z);
		float r_u = sqrtf(vxwf3.x * vxwf3.x + vxwf3.y * vxwf3.y + vxwf3.z * vxwf3.z) / ds;
		float r_v = sqrtf(uxwf3.x * uxwf3.x + uxwf3.y * uxwf3.y + uxwf3.z * uxwf3.z) / ds;

		if (r_u + r_v <= 1.0f)
			hit.intersected = true;
	}

	return hit;
}
