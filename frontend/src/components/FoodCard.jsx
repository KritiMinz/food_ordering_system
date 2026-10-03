function FoodCard({
  food,
  onAddToCart,
  onViewDetails,
  isFavorite,
  onToggleFavorite,
}) {
  const isAvailable = food.available !== false

  return (
    <div
      className={`bg-white rounded-2xl shadow-md overflow-hidden ${
        !isAvailable ? "opacity-90" : ""
      }`}
    >

      {/* Food Image + Favorite Button */}

      <div
        className={`h-48 flex items-center justify-center text-7xl relative ${
          isAvailable
            ? "bg-orange-100"
            : "bg-gray-200"
        }`}
      >
        {food.image}

        {/* Favorite */}

        <button
          onClick={() => onToggleFavorite(food.id)}
          className={`absolute top-3 right-3 w-11 h-11 rounded-full
                      flex items-center justify-center text-2xl
                      shadow-md transition
                      ${
                        isFavorite
                          ? "bg-red-100 text-red-600"
                          : "bg-white text-gray-400 hover:text-red-500"
                      }`}
          title={
            isFavorite
              ? "Remove from favorites"
              : "Add to favorites"
          }
        >
          {isFavorite ? "❤️" : "♡"}
        </button>
      </div>


      {/* Food Information */}

      <div className="p-6">

        <div className="flex justify-end mb-2">
  <button
    onClick={() => onToggleFavorite(food.id)}
    className="text-2xl"
    title={
      isFavorite
        ? "Remove from favorites"
        : "Add to favorites"
    }
  >
    {isFavorite ? "❤️" : "🤍"}
  </button>
</div>

        <h3 className="text-xl font-bold text-gray-800">
          {food.name}
        </h3>

        <p className="text-gray-500 mt-2">
          {food.description}
        </p>


        {/* Availability */}

        <div className="mt-4">

          {isAvailable ? (
            <span
              className="inline-block px-3 py-1 rounded-full
                         bg-green-100 text-green-700
                         text-sm font-semibold"
            >
              🟢 Available
            </span>
          ) : (
            <span
              className="inline-block px-3 py-1 rounded-full
                         bg-red-100 text-red-700
                         text-sm font-semibold"
            >
              ❌ Currently unavailable
            </span>
          )}

        </div>


        {/* Price + Buttons */}

        <div className="flex items-center justify-between mt-5">

          <span className="text-xl font-bold text-orange-600">
            ₹{food.price}
          </span>


          <div className="flex gap-2">

            {/* Details */}

            <button
              onClick={onViewDetails}
              className="border border-orange-600
                         text-orange-600 px-4 py-2
                         rounded-lg hover:bg-orange-50"
            >
              Details
            </button>


            {/* Add To Cart */}

            <button
              onClick={() => onAddToCart(food)}
              disabled={!isAvailable}
              className={
                isAvailable
                  ? "bg-orange-600 text-white px-4 py-2 rounded-lg hover:bg-orange-700"
                  : "bg-gray-300 text-gray-500 px-4 py-2 rounded-lg cursor-not-allowed"
              }
            >
              {isAvailable
                ? "Add to Cart"
                : "Unavailable"}
            </button>

          </div>

        </div>

      </div>
    </div>
  )
}

export default FoodCard