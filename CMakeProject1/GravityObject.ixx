#include "Object.h"
#include <btBulletDynamicsCommon.h>
export module GravityObject;

export class GravityObject : public Object
{
public:
	GravityObject(const std::shared_ptr<gps::Model3D>& model,btScalar hitbox_radius , btScalar mass) : 
		Object(model), 
		hitbox_radius(hitbox_radius),
		mass(mass)
	{
        sphere_shape = std::make_unique<btSphereShape>(btScalar(hitbox_radius));
      
        btTransform startTransform;
        startTransform.setIdentity();

        //rigidbody is dynamic if and only if mass is non zero, otherwise static
       

        btVector3 localInertia(0, 0, 0);
        if (isDynamic())
            sphere_shape->calculateLocalInertia(mass, localInertia);

        startTransform.setOrigin(btVector3(0, 0, 0));

        //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
		motion_state = std::make_unique<btDefaultMotionState>(startTransform);
		
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motion_state.get(), sphere_shape.get(), localInertia);
	
        hitbox = std::unique_ptr<btRigidBody>(new btRigidBody(rbInfo));
	}
	void update() override
	{
		//USING THE SUBCLASS METHOD IN ORDER TO PRESERVE STATE OF THE PHYSICS
		Object::setPosition(glm::vec3(hitbox->getWorldTransform().getOrigin().x(), hitbox->getWorldTransform().getOrigin().y(), hitbox->getWorldTransform().getOrigin().z()));		
		
		const auto bt_quart = hitbox->getWorldTransform().getRotation();
		
		const auto bt_axis = bt_quart.getAxis();
		const auto bt_angle = bt_quart.getAngle();

		//TODO: FIGURE OUT WHY DO I HAVE TO MULTIPLY WITH 180 FOR NORMAL BEHAVIOR????
		rotate_fixed(bt_angle * 180.0f, glm::vec3(bt_axis.z() , bt_axis.y() , bt_axis.x() ));
		
	}
	btRigidBody* getHitbox()
	{
		return hitbox.get();
	}
	btCollisionShape* getShape()
	{
		return sphere_shape.get();
	}

	void setPosition(const glm::vec3& pos) override
	{
		model_mat = glm::translate(glm::mat4(1.0f), pos);
		model_mat = glm::rotate(model_mat, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
		btTransform world;
		world.setIdentity();
		world.setOrigin(btVector3(pos.x, pos.y, pos.z));
		motion_state->setWorldTransform(world);
		hitbox->setMotionState(motion_state.get());
		hitbox->setWorldTransform(world);
	}
	void set_scale(glm::vec3 new_scale)
	{
		//TODO:
		model_mat = glm::scale(model_mat, new_scale);
	}
	bool isDynamic()
	{
		return ( mass != 0.f);
	}
private:
	btScalar mass;
	btScalar hitbox_radius;
  
	std::unique_ptr<btRigidBody> hitbox;
	std::unique_ptr<btCollisionShape> sphere_shape;
	std::unique_ptr<btDefaultMotionState> motion_state;
};