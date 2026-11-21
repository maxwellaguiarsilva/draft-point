//	unit-ray sphere evaluator returning world-metric hit distance and inclination cosine
hit_result evaluate_sphere( vec3 ray_origin, vec3 unit_direction, figure figure_item )
{
	float	radius = figure_item.parameters[ 0 ];
	vec3	local_center = figure_item.position - ray_origin;
	float	opposite_leg_squared = dot( local_center, local_center ) - square( figure_item.along );
	float	radius_squared = square( radius );
	if( opposite_leg_squared > radius_squared )
		return	no_hit( );
	float	cosine_incidence = sqrt( 1.0 - clamp( opposite_leg_squared / radius_squared, 0.0, 1.0 ) );
	return	hit_result( true, figure_item.along - radius * cosine_incidence, cosine_incidence );
}
