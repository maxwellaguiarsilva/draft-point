//	unit-ray capsule evaluator combining finite cylinder body and two hemisphere endcaps
hit_result evaluate_capsule( vec3 ray_origin, vec3 unit_direction, figure figure_item )
{
	float	radius = figure_item.parameters[ 4 ];
	vec3	axis = vec3_from( figure_item.parameters, 0u );
	float	half_length = figure_item.parameters[ 3 ];
	float	radius_squared = square( radius );
	vec3	local_center = figure_item.position - ray_origin;
	float	direction_along = dot( unit_direction, axis );
	float	center_along = dot( local_center, axis );
	vec3	perpendicular_direction = unit_direction - direction_along * axis;
	vec3	perpendicular_center = local_center - center_along * axis;
	float	perpendicular_direction_squared = dot( perpendicular_direction, perpendicular_direction );
	float	best_distance = 1e20;
	float	best_cosine = 0.0;
	bool	found = false;

	for( int branch_index = 0; branch_index < 3; ++branch_index )
	{
		bool	is_cylinder = branch_index == 0;
		if( is_cylinder && perpendicular_direction_squared <= 1e-12 )
			continue;
		float	cap_sign = branch_index == 1 ? -1.0 : 1.0;
		float	direction_scale = is_cylinder ? sqrt( perpendicular_direction_squared ) : 1.0;
		vec3	branch_local = is_cylinder
			? perpendicular_center
			: local_center + ( cap_sign * half_length ) * axis;
		float	branch_along = dot(
			 branch_local
			,is_cylinder ? perpendicular_direction / direction_scale : unit_direction
		);
		float	opposite_leg_squared = dot( branch_local, branch_local ) - square( branch_along );
		if( opposite_leg_squared > radius_squared )
			continue;
		float	cross_cosine = sqrt( 1.0 - clamp( opposite_leg_squared / radius_squared, 0.0, 1.0 ) );
		float	half_chord = radius * cross_cosine / direction_scale;
		float	center_distance = branch_along / direction_scale;
		for( int hit_index = 0; hit_index < 2; ++hit_index )
		{
			float	candidate = center_distance + ( hit_index == 0 ? -half_chord : half_chord );
			if( candidate <= 0.0 || candidate >= best_distance )
				continue;
			if( is_cylinder
				? abs( candidate * direction_along - center_along ) <= half_length
				: cap_sign * dot( candidate * unit_direction - branch_local, axis ) >= 0.0 )
			{
				best_distance = candidate;
				best_cosine = direction_scale * cross_cosine;
				found = true;
			}
		}
	}

	return	found ? hit_result( true, best_distance, best_cosine ) : no_hit( );
}
