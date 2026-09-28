function FoodCard({ food, onAddToCart, onViewDetails }) {
  return (
    <div className="bg-white rounded-2xl shadow-md overflow-hidden">

      <div className="h-48 bg-orange-100 flex items-center justify-center text-7xl">
        {food.image}
      </div>

      <div className="p-6">

        <h3 className="text-xl font-bold text-gray-800">
          {food.name}
        </h3>

        <p className="text-gray-500 mt-2">
          {food.description}
        </p>

        <div className="flex items-center justify-between mt-5">

          <span className="text-xl font-bold text-orange-600">
            ₹{food.price}
          </span>

          <div className="flex gap-2">

  <button
    onClick={onViewDetails}
    className="border border-orange-600 text-orange-600 px-4 py-2
               rounded-lg hover:bg-orange-50"
  >
    Details
  </button>

  <button
    onClick={() => onAddToCart(food)}
    className="bg-orange-600 text-white px-4 py-2 rounded-lg
               hover:bg-orange-700"
  >
    Add to Cart
  </button>

</div>

        </div>

      </div>
    </div>
  )
}

export default FoodCard
