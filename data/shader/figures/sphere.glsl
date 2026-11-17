//	unit-ray sphere evaluator returning world-metric hit distance and inclination cosine
hit_result evaluate_sphere( vec3 ray_origin, vec3 unit_direction, figure figure_item )
{
	float	radius = figure_item.parameters[ 0 ];
	vec3	local_center = figure_item.position - ray_origin;
	float	along = dot( local_center, unit_direction );
	float	opposite_leg_squared = dot( local_center, local_center ) - square( along );
	float	radius_squared = square( radius );
	if( opposite_leg_squared > radius_squared )
		return	no_hit( );
	float	cosine_incidence = sqrt( 1.0 - clamp( opposite_leg_squared / radius_squared, 0.0, 1.0 ) );
	float	half_chord = radius * cosine_incidence;
	float	near_distance = along - half_chord;
	float	far_distance = along + half_chord;
	if( far_distance <= 0.0 )
		return	no_hit( );
	return	hit_result( true, near_distance > 0.0 ? near_distance : far_distance, cosine_incidence );
}
