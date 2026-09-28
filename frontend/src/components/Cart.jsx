function Cart({ cart, onUpdateQuantity, onRemoveItem }) {
  const total = cart.reduce(
    (sum, item) => sum + item.price * item.quantity,
    0
  )

  if (cart.length === 0) {
    return (
      <div className="bg-white rounded-2xl shadow-md p-8 text-center">
        <div className="text-5xl mb-4">🛒</div>

        <h3 className="text-xl font-bold text-gray-800">
          Your cart is empty
        </h3>

        <p className="text-gray-500 mt-2">
          Add some delicious food to get started.
        </p>
      </div>
    )
  }

  return (
    <div className="bg-white rounded-2xl shadow-md p-6">

      <h2 className="text-2xl font-bold text-gray-800 mb-6">
        Your Cart
      </h2>

      <div className="space-y-5">

        {cart.map((item) => (
          <div
            key={item.id}
            className="flex items-center justify-between border-b pb-5"
          >

            <div className="flex items-center gap-4">

              <div className="text-4xl">
                {item.image}
              </div>

              <div>
                <h3 className="font-bold text-gray-800">
                  {item.name}
                </h3>

                <p className="text-orange-600 font-semibold">
                  ₹{item.price}
                </p>
              </div>

            </div>

            <div className="flex items-center gap-4">

              <button
                onClick={() =>
                  onUpdateQuantity(item.id, item.quantity - 1)
                }
                className="w-8 h-8 rounded-full bg-gray-200 hover:bg-gray-300"
              >
                -
              </button>

              <span className="font-semibold">
                {item.quantity}
              </span>

              <button
                onClick={() =>
                  onUpdateQuantity(item.id, item.quantity + 1)
                }
                className="w-8 h-8 rounded-full bg-gray-200 hover:bg-gray-300"
              >
                +
              </button>

              <button
                onClick={() => onRemoveItem(item.id)}
                className="text-red-500 hover:text-red-700 ml-3"
              >
                Remove
              </button>

            </div>

          </div>
        ))}

      </div>

      <div className="flex justify-between items-center mt-6 pt-4">

        <span className="text-xl font-bold text-gray-800">
          Total
        </span>

        <span className="text-2xl font-bold text-orange-600">
          ₹{total}
        </span>

      </div>

      <button
        className="w-full mt-6 bg-orange-600 text-white py-3 rounded-lg
                   font-semibold hover:bg-orange-700"
      >
        Proceed to Checkout
      </button>

    </div>
  )
}

export default Cart
