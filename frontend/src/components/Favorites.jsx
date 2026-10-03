function Favorites({
  foods,
  favorites,
  onAddToCart,
  onViewDetails,
  onToggleFavorite,
  onBack,
}) {
  const favoriteFoods = foods.filter((food) =>
    favorites.includes(food.id)
  )

  return (
    <section className="min-h-screen bg-gray-50 py-10 px-6">

      <div className="max-w-6xl mx-auto">

        {/* Header */}

        <div className="flex items-center justify-between mb-8">

          <div>
            <h2 className="text-3xl font-bold text-gray-800">
              ❤️ My Favorites
            </h2>

            <p className="text-gray-500 mt-2">
              Foods you have liked
            </p>
          </div>

          <button
            onClick={onBack}
            className="border border-orange-600
                       text-orange-600 px-5 py-2
                       rounded-lg hover:bg-orange-50"
          >
            ← Back to Menu
          </button>

        </div>


        {/* Empty State */}

        {favoriteFoods.length === 0 && (
          <div className="bg-white rounded-2xl shadow-md
                          p-12 text-center">

            <div className="text-6xl mb-4">
              💔
            </div>

            <h3 className="text-2xl font-bold text-gray-800">
              No Favorites Yet
            </h3>

            <p className="text-gray-500 mt-2">
              You haven't added any food to your favorites.
            </p>

            <button
              onClick={onBack}
              className="mt-6 bg-orange-600 text-white
                         px-6 py-3 rounded-lg
                         hover:bg-orange-700"
            >
              Explore Food
            </button>

          </div>
        )}


        {/* Favorite Foods */}

        {favoriteFoods.length > 0 && (
          <div className="grid grid-cols-1 md:grid-cols-2
                          lg:grid-cols-3 gap-6">

            {favoriteFoods.map((food) => {

              const isAvailable =
                food.available !== false

              return (
                <div
                  key={food.id}
                  className="bg-white rounded-2xl
                             shadow-md overflow-hidden"
                >

                  {/* Food Image */}

                  <div
                    className={`h-48 flex items-center
                                justify-center text-7xl ${
                      isAvailable
                        ? "bg-orange-100"
                        : "bg-gray-200"
                    }`}
                  >
                    {food.image}
                  </div>


                  {/* Food Information */}

                  <div className="p-6">

                    <div className="flex items-center
                                    justify-between">

                      <h3 className="text-xl font-bold
                                     text-gray-800">
                        {food.name}
                      </h3>

                      <button
                        onClick={() =>
                          onToggleFavorite(food.id)
                        }
                        className="text-2xl
                                   hover:scale-110
                                   transition-transform"
                        title="Remove from favorites"
                      >
                        ❤️
                      </button>

                    </div>

                    <p className="text-gray-500 mt-2">
                      {food.description}
                    </p>


                    <div className="flex items-center
                                    justify-between mt-5">

                      <span className="text-xl font-bold
                                       text-orange-600">
                        ₹{food.price}
                      </span>

                      <div className="flex gap-2">

                        <button
                          onClick={() =>
                            onViewDetails(food)
                          }
                          className="border border-orange-600
                                     text-orange-600 px-4 py-2
                                     rounded-lg
                                     hover:bg-orange-50"
                        >
                          Details
                        </button>

                        <button
                          onClick={() =>
                            onAddToCart(food)
                          }
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
            })}

          </div>
        )}

      </div>

    </section>
  )
}

export default Favorites
