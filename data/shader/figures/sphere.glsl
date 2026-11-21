{
	float	radius = figure_item.parameters[ 0 ];
	float	opposite_leg_squared = dot( local_center, local_center ) - square( along );
	float	radius_squared = square( radius );
	if( opposite_leg_squared <= radius_squared )
	{
		float	hit_distance = along - radius * sqrt( 1.0 - clamp( opposite_leg_squared / radius_squared, 0.0, 1.0 ) );
		result = hit_result( true
			,hit_distance
			,( hit_distance * direction - local_center ) / radius
		);
	}
}
