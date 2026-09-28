function FoodDetails({ food, onClose, onAddToCart, isFavorite, onToggleFavorite }) {
  if (!food) {
    return null
  }

  return (
    <div className="fixed inset-0 bg-black/50 flex items-center justify-center z-50 px-4">

      <div className="bg-white rounded-2xl shadow-xl max-w-lg w-full overflow-hidden">

        {/* Food Image */}
        <div className="h-64 bg-orange-100 flex items-center justify-center text-8xl">
          {food.image}
        </div>

        <div className="p-6">

          <div className="flex items-start justify-between">

            <div>
              <h2 className="text-3xl font-bold text-gray-800">
                {food.name}
              </h2>

              <p className="text-gray-500 mt-2">
                Category: {food.category}
              </p>
            </div>

            <button
              onClick={() => onToggleFavorite(food.id)}
              className="text-3xl"
            >
              {isFavorite ? "❤️" : "🤍"}
            </button>

          </div>

          <p className="text-gray-600 mt-5">
            {food.description}
          </p>

          <div className="flex items-center justify-between mt-6">

            <span className="text-2xl font-bold text-orange-600">
              ₹{food.price}
            </span>

            <button
              onClick={() => onAddToCart(food)}
              className="bg-orange-600 text-white px-6 py-3 rounded-lg
                         font-semibold hover:bg-orange-700"
            >
              Add to Cart
            </button>

          </div>

          <button
            onClick={onClose}
            className="w-full mt-4 border border-gray-300 py-3 rounded-lg
                       text-gray-700 hover:bg-gray-100"
          >
            Close
          </button>

        </div>

      </div>

    </div>
  )
}

export default FoodDetails